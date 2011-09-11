<?php

/**
 * @file
 * Contains the EntityFieldHandlerHelper class.
 */

/**
 * Helper class containing static implementations of common field handler methods.
 *
 * Used by the generic entity field handlers to avoid code duplication.
 */
class EntityFieldHandlerHelper {

  /**
   * Add the field for the entity ID (if necessary).
   */
  public static function query($handler) {
    // Some of the parent handlers might require this.
    $handler->field_alias = $handler->real_field;
    // Add the field if the query plugin implements an add_field() method, just
    // like the default plugin.
    if (method_exists($handler->query, 'add_field')) {
      $handler->table_alias = $base_table = $handler->view->base_table;
      $handler->base_field = $handler->view->base_field;

      if (!empty($handler->relationship)) {
        foreach ($handler->view->relationship as $relationship) {
          if ($relationship->alias == $handler->relationship) {
            $base_table = $relationship->definition['base'];
            $handler->table_alias = $relationship->alias;

            $table_data = views_fetch_data($base_table);
            $handler->base_field = $table_data['table']['base']['field'];
          }
        }
      }

      $handler->query->add_field($handler->table_alias, $handler->base_field, '');
    }
  }

  /**
   * Load the entities for all rows that are about to be displayed.
   */
  public static function pre_render($handler, &$values) {
    if (!empty($values)) {
      list($handler->entity_type, $handler->entities) = $handler->query->get_result_entities($values, !empty($handler->relationship) ? $handler->relationship : NULL);
    }
  }

  /**
   * Overridden to use a metadata wrapper (if necessary).
   */
  public static function get_value($handler, $values, $field = NULL, $default = NULL) {
    $field = isset($field) ? $field : $handler->real_field;
    if (isset($values->$field)) {
      return $values->$field;
    }
    if (!isset($handler->entities[$handler->view->row_index])) {
      return $default;
    }
    $entity = $handler->entities[$handler->view->row_index];
    $wrapper = entity_metadata_wrapper($handler->entity_type, $entity);
    return isset($wrapper->$field) ? $wrapper->$field->value(array('identifier' => TRUE)) : $default;
  }

}
