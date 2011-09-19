<?php

/**
 * @file
 * Contains the entity_views_handler_field_text class.
 */

/**
 * A handler to display text data.
 *
 * Overrides the default Views handler to retrieve the data from an entity.
 *
 * @ingroup views_field_handlers
 */
class entity_views_handler_field_text extends views_handler_field {

  /**
   * Stores the entity type which is loaded by this field.
   */
  public $entity_type;

  /**
   * Stores all entities which are in the result.
   */
  public $entities = array();

  /**
   * Stores the result entities' metadata wrappers.
   */
  public $wrappers = array();

  /**
   * The base name of the field, without property selector.
   */
  public $base_field;

  /**
   * A value cache used in get_value() to make list fields work properly.
   */
  public $_cache = array();

  /**
   * Overridden to add the field for the entity ID (if necessary).
   */
  public function query() {
    EntityFieldHandlerHelper::query($this);
  }

  /**
   * Adds a click-sort to the query.
   */
  public function click_sort($order) {
    EntityFieldHandlerHelper::click_sort($this, $order);
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
    return $this->sanitize_value($value, 'xss');
  }

}
