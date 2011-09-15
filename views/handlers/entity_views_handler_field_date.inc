<?php

/**
 * @file
 * Contains the entity_views_handler_field_date class.
 */

/**
 * A handler to provide proper displays for dates.
 *
 * Overrides the default Views handler to retrieve the data from an entity.
 *
 * @ingroup views_field_handlers
 */
class entity_views_handler_field_date extends views_handler_field_date {

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
   * Provide options for this handler.
   */
  public function option_definition() {
    return parent::option_definition() + EntityFieldHandlerHelper::option_definition($this);
  }

  /**
   * Provide a options form for this handler.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    EntityFieldHandlerHelper::options_form($this, $form, $form_state);
  }

  /**
   * Render the field.
   *
   * @param $values
   *   The values retrieved from the database.
   */
  public function render($values) {
    return EntityFieldHandlerHelper::render($this, $values);
  }

  /**
   * Render a single field value.
   */
  public function render_single_value($value, $values) {
    $values->_entity_properties[$this->field_alias] = $value;
    return parent::render($values);
  }

}
