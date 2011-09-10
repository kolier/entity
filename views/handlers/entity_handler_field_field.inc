<?php

/**
 * @file
 * Contains the entity_handler_field_field class.
 */

/**
 * A handler to provide proper displays for Field API fields.
 *
 * @ingroup views_field_handlers
 */
// Class contents are mostly copied from views_handler_field_entity.
class entity_handler_field_field extends views_handler_field_field {

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
   * Overridden to get the entity and put it where it is assumed to be.
   */
  public function get_value($values, $field = NULL) {
    if (!isset($this->entities[$this->view->row_index])) {
      return NULL;
    }
    $entity = $this->entities[$this->view->row_index];
    $values->_field_data[$this->field_alias]['entity'] = $entity;
    $values->_field_data[$this->field_alias]['entity_type'] = $this->entity_type;

    return parent::get_value($values, $field);
  }

}
