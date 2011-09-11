<?php

/**
 * @file
 * Contains the entity_handler_field_entity class.
 */

/**
 * A handler to provide proper displays for entities.
 *
 * @ingroup views_field_handlers
 */
// Class contents are mostly copied from views_handler_field_entity.
class entity_handler_field_entity extends views_handler_field_entity {

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
    $ret = EntityFieldHandlerHelper::get_value($this, $values, $field);
    // @todo Should be handled by pre_render(), either with a relationship
    //   (probably not possible) or manually (but with multiple load).
    if (!is_object($ret)) {
      // @todo Is this all the list handling we should do? I guess not …
      while (is_array($ret)) $ret = reset($ret);
      if (is_string($ret) || is_integer($ret)) {
        $ret = entity_load_single($this->definition['entity type'], $ret);
      }
    }
    return $ret;
  }

  public function option_definition() {
    $options = parent::option_definition();

    $options['display'] = array('default' => 'label');
    $options['link_to_entity'] = array('default' => TRUE);
    $options['view_mode'] = array('default' => 'default');

    return $options;
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

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
    $entity = $this->get_value($values);
    if (!$entity) {
      return '';
    }
    $type = $this->definition['entity type'];

    if ($this->options['display'] == 'view') {
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

    if ($this->options['link_to_entity']) {
      $uri = entity_uri($type, $entity);
      if ($uri) {
        if ($type == 'file' && !is_array($uri)) {
          $uri = array(
            'path' => file_create_url($uri),
            'options' => array(),
          );
        }
        $uri['options']['html'] = TRUE;
        $value = l($value, $uri['path'], $uri['options']);
      }
    }

    return $value;
  }

}
