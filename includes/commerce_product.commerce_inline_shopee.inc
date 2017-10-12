<?php

/**
 * @file
 * Defines the inline entity form controller for Commerce Products.
 */

class CommerceProductCommerceInlineShopeeController extends CommerceProductInlineEntityFormController {

  /**
   * Overrides EntityInlineEntityFormController::tableFields().
   */
  public function tableFields($bundles) {
    $fields = array();
    $fields['title'] = array(
      'type' => 'property',
      'label' => t('Name'),
      'weight' => 1,
    );
    $fields['sku'] = array(
      'type' => 'property',
      'label' => t('SKU'),
      'weight' => 2,
    );
    
    $weight = 3;
    // If only one product type is allowed, its fields can be used as columns.
    if (count($bundles) == 1) {
      $bundle = reset($bundles);

      foreach (field_info_instances('commerce_product', $bundle) as $field_name => $instance) {
        $field = field_info_field($field_name);

        // If the product has an imagefield, show it.
        if ($field['type'] == 'image') {
          // Determine the correct image style to use.
          $image_style = 'thumbnail';
          if (!empty($instance['widget']['settings']['preview_image_style'])) {
            $image_style = $instance['widget']['settings']['preview_image_style'];
          }

          $fields[$field_name] = array(
            'type' => 'field',
            'label' => $instance['label'],
            'formatter' => 'image',
            'settings' => array('image_style' => $image_style),
            'delta' => 0,
            'weight' => $weight,
          );
          // Don't add any other imagefields. One is enough.
          //break;
        }
        else {
          $fields[$field_name] = array(
            'type' => 'field',
            'label' => $instance['label'],
            'formatter' => $field['type'],
            'weight' => $weight,
          );
        }
        $weight++;
      }

      // If the type has up to 3 attributes, show them instead of the title field.
      $attributes = $this->attributes($bundle);
      if (count($attributes) <= 3) {
        $fields['title']['visible'] = FALSE;

        foreach ($attributes as $field_name => $attribute) {
          $field_type = field_info_field_types($attribute['field']['type']);
          // Override the default formatter for taxonomy_term_reference.
          if ($field_type['default_formatter'] == 'taxonomy_term_reference_link') {
            $field_type['default_formatter'] = 'taxonomy_term_reference_plain';
          }

          $weight = -3;
          $fields[$field_name] = array(
            'type' => 'field',
            'label' => $attribute['instance']['label'],
            'formatter' => $field_type['default_formatter'],
            'weight' => ++$weight,
          );
        }
      }
    }

    $fields['commerce_price'] = array(
      'type' => 'field',
      'label' => t('Price'),
      'formatter' => 'commerce_price_formatted_amount',
      'weight' => 99,
    );

    return $fields;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityForm().
   */
  public function entityForm($entity_form, &$form_state) {
    global $user;

    // Get the labels (product / variation).
    $labels = $this->labels();

    $product = $entity_form['#entity'];
    $extra_fields = field_info_extra_fields('commerce_product', $product->type, 'form');

    //Add the previous Price and Stock
    if (!isset($product->commerce_price) && !empty($form_state['last_entry'])) {
      foreach ($form_state['last_entry'] as $key => $field) {
        if (is_array($field) && !empty($field)) {
          $product->$key = $field;
        }
      }
    }

    // Assign newly created products to the current user.
    if (empty($product->product_id)) {
      $product->uid = $user->uid;
    }

    $entity_form['product_attributes'] = array(
      '#type' => 'fieldset',
      '#title' => t('Attributes'),
      '#attributes' => array('class' => array('container-inline', 'ief-product-attributes', 'ief-entity-fieldset')),
    );
    $entity_form['product_details'] = array(
      '#type' => 'fieldset',
      '#title' => t('Details'),
      '#attributes' => array('class' => array('ief-product-details', 'ief-entity-fieldset')),
    );

    $entity_form['sku'] = array(
      '#type' => 'textfield',
      '#title' => t('SKU'),
      '#description' => t('Supply a unique identifier using letters, numbers, hyphens, and underscores. Commas may not be used.'),
      '#default_value' => $product->sku,
      '#maxlength' => 255,
      '#required' => TRUE,
      '#fieldset' => 'product_details',
      '#weight' => $extra_fields['sku']['weight'],
    );
    $entity_form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#default_value' => $product->title,
      '#maxlength' => 255,
      '#required' => TRUE,
      '#fieldset' => 'product_details',
      // The label might be missing if the Title module has replaced it.
      '#weight' => !empty($extra_fields['title']) ? $extra_fields['title']['weight'] : -9,
    );

    // Attach fields.
    $langcode = entity_language('commerce_product', $product);
    field_attach_form('commerce_product', $product, $entity_form, $form_state, $langcode);

    // Hide or disable the SKU field if it is auto-generated by Commerce AutoSKU.
    if (module_exists('commerce_autosku') && $settings = commerce_autosku_get_settings($product)) {
      $entity_form['sku']['#required'] = FALSE;
      $entity_form['sku']['#disabled'] = TRUE;
      if ($settings['advanced']['hide_sku']) {
        $entity_form['sku']['#access'] = FALSE;
      }
      else {
        $entity_form['sku']['#description'] = t('Will be auto-generated when the form is saved.');
      }
    }
    // Hide the title field if it is auto-generated.
    if ($this->settings['autogenerate_title']) {
      $entity_form['title']['#required'] = FALSE;
      $entity_form['title']['#access'] = FALSE;
      // Hide the replacement field added by the Title module as well.
      if (module_exists('title')) {
        $title_field = title_field_replacement_info('commerce_product', 'title');
        if ($title_field) {
          $title_field_name = $title_field['field']['field_name'];
          if (isset($entity_form[$title_field_name])) {
            $entity_form[$title_field_name]['#access'] = FALSE;
            $entity_form[$title_field_name]['#required'] = FALSE;
          }
        }
      }
    }

    // Arrange attributes.
    $attributes = $this->attributes($product->type);
    if (empty($attributes)) {
      // Hide the fieldset, it will be empty.
      $entity_form['product_attributes']['#access'] = FALSE;
    }
    else {
      foreach ($attributes as $field_name => $attribute) {
        $entity_form[$field_name]['#fieldset'] = 'product_attributes';
      }
    }

    // Arrange non-attribute fields.
    foreach (field_info_instances('commerce_product', $product->type) as $name => $instance) {
      $field_name = $instance['field_name'];
      $field = field_info_field($field_name);
      if (!isset($attributes[$field_name])) {
        $entity_form[$field_name]['#fieldset'] = 'product_details';
      }
    }

    return $entity_form;
  }
}