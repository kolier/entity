<?php

/**
 * @file
 * Contains the entity_views_handler_field_entity class.
 */

/**
 * A handler to provide proper displays for entities.
 *
 * @ingroup views_field_handlers
 */
class entity_views_handler_field_entity extends views_handler_field_entity {

  /**
   * Overridden to add the field for the entity ID (if necessary).
   */
  public function query() {
    EntityFieldHandlerHelper::query($this);
  }

  /**
   * Load the entities for all rows that are about to be displayed.
   */
  public function pre_render(&$values) {
    parent::pre_render($values);
  }

  /**
   * Overridden to use a metadata wrapper.
   */
  public function get_value($values, $field = NULL) {
    return EntityFieldHandlerHelper::get_value($this, $values, $field);
  }

  public function option_definition() {
    $options = parent::option_definition();
    $options += EntityFieldHandlerHelper::option_definition($this);

    $options['display'] = array('default' => 'label');
    $options['link_to_entity']['default'] = TRUE;
    $options['view_mode'] = array('default' => 'default');

    return $options;
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    EntityFieldHandlerHelper::options_form($this, $form, $form_state);
    // We want a different form field at a different place.
    unset($form['link_to_entity']);

    $options = array(
      'label' => t('Show entity label'),
      'id' => t('Show entity ID'),
      'view' => t('Show complete entity'),
    );
    $form['display'] = array(
      '#type' => 'select',
      '#title' => t('Display'),
      '#description' => t('Decide how this field will be displayed.'),
      '#options' => $options,
      '#default_value' => $this->options['display'],
    );
    $form['link_to_entity'] = array(
      '#type' => 'checkbox',
      '#title' => t('Link to entity'),
      '#description' => t('Link this field to the entity.'),
      '#default_value' => $this->options['link_to_entity'],
      '#dependency' => array('edit-options-display' => array('label', 'id')),
    );

    // Stolen from entity_plugin_row_entity_view.
    $entity_info = entity_get_info($this->definition['entity type']);
    $options = array();
    if (!empty($entity_info['view modes'])) {
      foreach ($entity_info['view modes'] as $mode => $settings) {
        $options[$mode] = $settings['label'];
      }
    }

    if (count($options) > 1) {
      $form['view_mode'] = array(
        '#type' => 'select',
        '#options' => $options,
        '#title' => t('View mode'),
        '#default_value' => $this->options['view_mode'],
      );
    }
    else {
      $form['view_mode'] = array(
        '#type' => 'value',
        '#value' => $options ? key($options) : 'default',
      );
    }
  }

  public function render($values) {
    return EntityFieldHandlerHelper::render($this, $values);
  }

  /**
   * Render a single field value.
   */
  public function render_single_value($entity, $values) {
    if (!$entity) {
      return '';
    }
    $type = $this->definition['entity type'];

    if ($this->options['display'] === 'view') {
      return entity_view($type, $entity, $this->options['view_mode']);
    }

    if ($this->options['display'] == 'label') {
      $value = entity_label($type, $entity);
    }
    // Either $options[display] == 'id', or we have no label.
    if (empty($value)) {
      $value = entity_id($type, $entity);
    }
    $value = $this->sanitize_value($value);

    return $value;
  }

}
