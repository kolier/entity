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
class entity_handler_field_options extends views_handler_field {

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
    EntityFieldHandlerHelper::pre_render($this, $values);
  }

  /**
   * Overridden to use a metadata wrapper.
   */
  public function get_value($values, $field = NULL) {
    return EntityFieldHandlerHelper::get_value($this, $values, $field);
  }

  /**
   * Specifies the options this handler uses.
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['format_name'] = array('default' => TRUE);
    return $options;
  }

  /**
   * Returns an option form for setting this handler's options.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['format_name'] = array(
      '#title' => t('Use human-readable name'),
      '#type' => 'checkbox',
      '#description' => t("If this is checked, the values' names will be displayed instead of their internal identifiers."),
      '#default_value' => $this->options['format_name'],
      '#weight' => -5,
    );
  }


  public function render($values) {
    $value = $this->get_value($values);

    if ($this->options['format_name'] && isset($this->definition['options'][$value])) {
      $value = $this->definition['options'][$value];
    }

    return $this->sanitize_value($value);
  }

}
