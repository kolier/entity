<?php

/**
 * @file
 * Contains the entity_handler_field_boolean class.
 */

/**
 * A handler to provide proper displays for booleans.
 *
 * Allows for display of true/false, yes/no, on/off, enabled/disabled.
 *
 * Definition terms:
 *   - output formats: An array where the first entry is displayed on boolean true
 *      and the second is displayed on boolean false. An example for sticky is:
 *      @code
 *      'output formats' => array(
 *        'sticky' => array(t('Sticky'), ''),
 *      ),
 *      @endcode
 *
 * @ingroup views_field_handlers
 */
// Class contents are mostly copied from views_handler_field_entity.
class entity_handler_field_boolean extends views_handler_field_boolean {

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
      return FALSE;
    }
    $field = isset($field) ? $field : $this->real_field;
    $entity = $this->entities[$this->view->row_index];
    $wrapper = entity_metadata_wrapper($this->entity_type, $entity);
    return isset($wrapper->$field) ? $wrapper->$field->value() : FALSE;
  }

}
