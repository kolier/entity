<?php

/**
 * @file
 * Contains the entity_handler_field_duration class.
 */

/**
 * A handler to provide proper displays for durations.
 *
 * @ingroup views_field_handlers
 */
// Class contents are mostly copied from views_handler_field_entity.
class entity_handler_field_duration extends views_handler_field {

  /**
   * Stores the entity type which is loaded by this field.
   */
  public $entity_type;

  /**
   * Stores all entites which are in the result.
   */
  public $entities;

  /**
   * The base field of the entity type assosiated with this field.
   */
  public $base_field;

  /**
   * Overriden to add the field for the entity id.
   */
  function query() {
    // @todo Figure out exactly what relationship data we got here, and what we
    //   need to do about it. (Anything at all?)
  }

  /**
   * Load the entities for all rows that are about to be displayed.
   */
  function pre_render(&$values) {
    parent::pre_render($values);
    if (!empty($values)) {
      list($this->entity_type, $this->entities) = $this->query->get_result_entities($values, !empty($this->relationship) ? $this->relationship : NULL, $this->field_alias);
    }
  }

  /**
   * Overridden to use a metadata wrapper.
   */
  function get_value($values, $field = NULL) {
    if (!isset($this->entities[$this->view->row_index])) {
      return FALSE;
    }
    $field = isset($field) ? $field : $this->real_field;
    $entity = $this->entities[$this->view->row_index];
    $wrapper = entity_metadata_wrapper($this->entity_type, $entity);
    return isset($wrapper->$field) ? $wrapper->$field->value() : FALSE;
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['format_interval'] = array('default' => TRUE);
    $options['granularity'] = array('default' => 2);
    $options['prefix'] = array('default' => '', 'translatable' => TRUE);
    $options['suffix'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['format_interval'] = array(
      '#type' => 'checkbox',
      '#title' => t('Format interval'),
      '#description' => t('If checked, the value will be formatted as a time interval. Otherwise, just the number of seconds will be displayed.'),
      '#default_value' => $this->options['format_interval'],
    );
    $form['granularity'] = array(
      '#type' => 'textfield',
      '#title' => t('Granularity'),
      '#default_value' => $this->options['granularity'],
      '#description' => t('Specify how many different units to display.'),
      '#dependency' => array('edit-options-format-interval' => array(TRUE)),
      '#size' => 2,
    );
    $form['prefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Prefix'),
      '#default_value' => $this->options['prefix'],
      '#description' => t('Text to put before the duration text.'),
    );
    $form['suffix'] = array(
      '#type' => 'textfield',
      '#title' => t('Suffix'),
      '#default_value' => $this->options['suffix'],
      '#description' => t('Text to put after the duration text.'),
    );
  }

  function render($values) {
    $value = $this->get_value($values);

    if ($this->options['format_interval']) {
      $value = format_interval($value, (int) $this->options['granularity']);
    }

    return $this->sanitize_value($this->options['prefix'], 'xss')
      . $this->sanitize_value($value)
      . $this->sanitize_value($this->options['suffix'], 'xss');
  }

}
