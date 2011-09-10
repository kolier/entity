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

}
