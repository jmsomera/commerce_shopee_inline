<?php

/**
 * @file
 * Defines the base inline entity form controller.
 */

class EntityCommerceInlineShopeeController extends EntityInlineEntityFormController {

  /*protected $entityType;
  public $settings;

  public function __construct($entityType, array $settings) {
    $this->entityType = $entityType;
    $this->settings = $settings + $this->defaultSettings();
  }*/

  /**
   * Returns an array of css filepaths for the current entity type, keyed
   * by theme name.
   *
   * If provided, the "base" CSS file is included for all themes.
   * If a CSS file matching the current theme exists, it will also be included.
   *
   * @code
   * return array(
   *   'base' => drupal_get_path('module', 'test_module') . '/css/inline_entity_form.base.css',
   *   'seven' => drupal_get_path('module', 'test_module') . '/css/inline_entity_form.seven.css',
   * );
   * @endcode
   */

  /**
   * Returns an array of default settings in the form of key => value.
   */
  public function defaultSettings() {
    $defaults = array();
    $defaults['allow_new'] = TRUE;
    $defaults['allow_existing'] = FALSE;
    $defaults['match_operator'] = 'CONTAINS';
    $defaults['allow_clone'] = FALSE;
    $defaults['delete_references'] = FALSE;
    $defaults['multi_edit'] = FALSE;
    $labels = $this->defaultLabels();
    $defaults['override_labels'] = FALSE;
    $defaults['label_singular'] = $labels['singular'];
    $defaults['label_plural'] = $labels['plural'];

    return $defaults;
  }

  /**
   * Returns the settings form for the current entity type.
   *
   * The settings form is embedded into the IEF widget settings form.
   * Settings are later injected into the controller through $this->settings.
   *
   * @param $field
   *   The definition of the reference field used by IEF.
   * @param $instance
   *   The definition of the reference field instance.
   */
  public function settingsForm($field, $instance) {
    $labels = $this->labels();
    $states_prefix = 'instance[widget][settings][type_settings]';

    $form = array();
    $form['allow_new'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to add new @label.', array('@label' => $labels['plural'])),
      '#default_value' => $this->settings['allow_new'],
    );
    $form['allow_existing'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to add existing @label.', array('@label' => $labels['plural'])),
      '#default_value' => $this->settings['allow_existing'],
    );
    $form['match_operator'] = array(
      '#type' => 'select',
      '#title' => t('Autocomplete matching'),
      '#default_value' => $this->settings['match_operator'],
      '#options' => array(
        'STARTS_WITH' => t('Starts with'),
        'CONTAINS' => t('Contains'),
      ),
      '#description' => t('Select the method used to collect autocomplete suggestions. Note that <em>Contains</em> can cause performance issues on sites with thousands of nodes.'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $states_prefix . '[allow_existing]"]' => array('checked' => TRUE),
        ),
      ),
    );
    // The single widget doesn't offer autocomplete functionality.
    if ($instance['widget']['type'] == 'inline_entity_form_single') {
      $form['allow_existing']['#access'] = FALSE;
      $form['match_operator']['#access'] = FALSE;
    }

    $form['allow_clone'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to clone @label.', array('@label' => $labels['plural'])),
      '#default_value' => isset($this->settings['allow_clone'])? $this->settings['allow_clone'] : FALSE,
    );
    $form['multi_edit'] = array(
      '#type' => 'checkbox',
      '#title' => t('Multi-edit?'),
      '#default_value' => $this->settings['multi_edit'],
      '#description' => t('Automatically show the edit forms for all objects.'),
    );
    $form['delete_references'] = array(
      '#type' => 'checkbox',
      '#title' => t('Delete referenced @label when the parent entity is deleted.', array('@label' => $labels['plural'])),
      '#default_value' => $this->settings['delete_references'],
    );
    $form['override_labels'] = array(
      '#type' => 'checkbox',
      '#title' => t('Override labels'),
      '#default_value' => $this->settings['override_labels'],
    );
    $form['label_singular'] = array(
      '#type' => 'textfield',
      '#title' => t('Singular label'),
      '#default_value' => $this->settings['label_singular'],
      '#states' => array(
        'visible' => array(
          ':input[name="' . $states_prefix . '[override_labels]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['label_plural'] = array(
      '#type' => 'textfield',
      '#title' => t('Plural label'),
      '#default_value' => $this->settings['label_plural'],
      '#states' => array(
        'visible' => array(
          ':input[name="' . $states_prefix . '[override_labels]"]' => array('checked' => TRUE),
        ),
      ),
    );

    return $form;
  }
}