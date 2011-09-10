<?php

/**
 * @file
 * Contains the entity_handler_field_date class.
 */

/**
 * A handler to provide proper displays for dates.
 *
 * @ingroup views_field_handlers
 */
// Class contents are mostly copied from views_handler_field_entity.
class entity_handler_field_date extends views_handler_field_date {

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
      list($this->entity_type, $this->entities) = $this->query->get_result_entities($values, !empty($this->relationship) ? $this->relationship : NULL);
    }
  }

  /**
   * Overridden to use a metadata wrapper.
   */
  function get_value($values, $field = NULL) {
    if (!isset($this->entities[$this->view->row_index])) {
      return NULL;
    }
    $field = isset($field) ? $field : $this->real_field;
    $entity = $this->entities[$this->view->row_index];
    $wrapper = entity_metadata_wrapper($this->entity_type, $entity);
    return isset($wrapper->$field) ? $wrapper->$field->value() : NULL;
  }

}
