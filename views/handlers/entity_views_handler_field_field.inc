<?php

/**
 * @file
 * Contains the entity_views_handler_field_field class.
 */

/**
 * A handler to provide proper displays for Field API fields.
 *
 * Overrides the default Views handler to retrieve the data from an entity.
 *
 * @ingroup views_field_handlers
 */
class entity_views_handler_field_field extends views_handler_field_field {

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
   * Return TRUE if the user has access to view this field.
   */
  public function access() {
    return field_access('view', $this->field_info, $this->definition['entity type']);
  }

  /**
   * Overridden to add the field for the entity ID (if necessary).
   */
  public function query($use_groupby = FALSE) {
    EntityFieldHandlerHelper::query($this);
  }

  /**
   * Adds a click-sort to the query.
   */
  public function click_sort($order) {
    EntityFieldHandlerHelper::click_sort($this, $order);
  }

  /**
   * Override so it doesn't do any harm (or, anything at all).
   */
  public function post_execute(&$values) {

  }

  /**
   * Load the entities for all rows that are about to be displayed.
   */
  public function pre_render(&$values) {
    parent::pre_render($values);
    EntityFieldHandlerHelper::pre_render($this, $values);
  }

  /**
   * Overridden to get the items our way.
   */
  public function get_items($values) {
    // Only try to render the field if it is even present on this bundle.
    // Otherwise, field_view_field() will trigger a fatal.
    // The $this->get_value() call is actually necessary in any case, to fill
    // some $values keys required by set_items().
    $entity = $this->get_value($values, 'entity');
    list (, , $bundle) = entity_extract_ids($this->entity_type, $entity);
    if (field_info_instance($this->entity_type, $this->definition['field_name'], $bundle)) {
      return $this->set_items($values, $this->view->row_index);
    }
    return array();
  }

  /**
   * Overridden to get the entity and put it where it is assumed to be.
   */
  public function get_value($values, $field = NULL) {
    if (!isset($this->entities[$this->view->row_index])) {
      return NULL;
    }
    $entity = $this->entities[$this->view->row_index];
    if (empty($values->_field_data[$this->field_alias]['entity'])) {
      $values->_field_data[$this->field_alias]['entity'] = $entity;
      $values->_field_data[$this->field_alias]['entity_type'] = $this->entity_type;
    }
    if ($field == 'entity') {
      return $entity;
    }

    return parent::get_value($values, $field);
  }

}
