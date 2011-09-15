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
// Class contents are mostly copied from views_handler_field_entity.
class EntityFieldHandlerHelper {

  /**
   * Provide appropriate default options for a handler.
   */
  public static function option_definition($handler) {
    if (entity_property_list_extract_type($handler->definition['type'])) {
      $options['list']['contains']['mode'] = array('default' => 'collapse');
      $options['list']['contains']['glue'] = array('default' => ', ');
    }
    $options['link_to_entity'] = array('default' => FALSE);

    return $options;
  }

  /**
   * Provide an appropriate default option form for a handler.
   */
  public static function options_form($handler, &$form, &$form_state) {
    if (entity_property_list_extract_type($handler->definition['type'])) {
      $form['list']['mode'] = array(
        '#type' => 'select',
        '#title' => t('List handling'),
        '#options' => array(
          'collapse' => t('Concatenate values using a seperator'),
          'first' => t('Show first (if present)'),
          'count' => t('Show item count'),
        ),
        '#default_value' => $handler->options['list']['mode'],
      );
      $form['list']['glue'] = array(
        '#type' => 'textfield',
        '#title' => t('List seperator'),
        '#default_value' => $handler->options['list']['glue'],
        '#dependency' => array('edit-options-list-mode' => array('collapse')),
      );
    }
    $form['link_to_entity'] = array(
      '#type' => 'checkbox',
      '#title' => t('Link this field to the result item'),
      '#description' => t('This will override any other link you have set.'),
      '#default_value' => $handler->options['link_to_entity'],
    );
  }

  /**
   * Add the field for the entity ID (if necessary).
   */
  public static function query($handler) {
    // Some of the parent handlers might require this.
    $handler->field_alias = $handler->real_field;
    // Add the field if the query plugin implements an add_field() method, just
    // like the default query plugin.
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
    if (isset($values->_entity_properties[$field])) {
      return $values->_entity_properties[$field];
    }
    if (!isset($handler->entities[$handler->view->row_index])) {
      return $default;
    }
    $entity = $handler->entities[$handler->view->row_index];
    if ($field === 'entity object') {
      return $entity;
    }
    $wrapper = entity_metadata_wrapper($handler->entity_type, $entity);
    return isset($wrapper->$field) ? $wrapper->$field->value(array('identifier' => TRUE)) : $default;
  }

  /**
   * Render the field.
   *
   * Implements the entity link functionality and list handling. Basic handling
   * of the single values is delegated back to the field handler.
   *
   * @param $values
   *   The values retrieved from the database.
   */
  public static function render($handler, $values) {
    $value = $handler->get_value($values);
    if (is_array($value)) {
      return self::render_list($handler, $value, $values);
    }
    return self::render_entity_link($handler, $value, $values);
  }

  /**
   * Render a list of values.
   */
  public static function render_list($handler, $value, $values) {
    // Allow easy overriding of this behaviour in the specific field handler.
    if (method_exists($handler, 'render_list')) {
      return $handler->render_list($value, $values);
    }
    if (isset($handler->options['list']['mode'])) {
      if ($handler->options['list']['mode'] == 'first') {
        $value = count($value) ? array_shift($value) : NULL;
        if (is_array($value)) {
          return self::render_entity_link($handler, $value, $values);
        }
        elseif (isset($value)) {
          return self::render_entity_link($handler, $value, $values);
        }
        return NULL;
      }
      if ($handler->options['list']['mode'] == 'count') {
        return count($value);
      }
    }
    $vs = array();
    foreach ($value as $v) {
      $v = is_array($v) ? self::render_list($handler, $v, $values) : self::render_entity_link($handler, $v, $values);
      if ($v) {
        $vs[] = $v;
      }
    }
    $glue = isset($handler->options['list']['glue']) ? $handler->options['list']['glue'] : ', ';
    return implode($glue, $vs);
  }

  /**
   * Render a value as a link to the entity, if applicable.
   */
  public static function render_entity_link($handler, $value, $values) {
    // Allow easy overriding of this behaviour in the specific field handler.
    if (method_exists($handler, 'render_entity_link')) {
      return $handler->render_entity_link($value, $values);
    }
    $render = self::render_single_value($handler, $value, $values);
    if (!$handler->options['link_to_entity']) {
      return $render;
    }
    $entity = $handler->get_value($values, 'entity object');
    if (is_object($entity) && ($url = entity_uri($handler->entity_type, $entity))) {
      // We LOVE our core bugs! (http://drupal.org/node/1057242)
      if ($handler->entity_type === 'file' && !is_array($url)) {
        $url = array(
          'path' => file_create_url($url),
          'options' => array(),
        );
      }
      return l($render, $url['path'], array('html' => TRUE) + $url['options']);
    }
    return $render;
  }

  /**
   * Render a single value.
   */
  public static function render_single_value($handler, $value, $values) {
    // Try to use the method in the specific field handler.
    if (method_exists($handler, 'render_single_value')) {
      return $handler->render_single_value($value, $values);
    }
    // Lame fallback in case the field handler doesn't provide the method.
    return is_scalar($value) ? check_plain($value) : nl2br(check_plain(print_r($value, TRUE)));
  }

}
