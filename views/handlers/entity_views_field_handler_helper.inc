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
      '#title' => t('Link this field to its entity'),
      '#description' => t("When using this, you should not set any other link on the field."),
      '#default_value' => $handler->options['link_to_entity'],
    );
  }

  /**
   * Add the field for the entity ID (if necessary).
   */
  public static function query($handler) {
    // Some of the parent handlers might require this.
    $handler->field_alias = $handler->real_field;
    $handler->base_field = ltrim(substr($handler->real_field, strrpos($handler->real_field, ':')), ':');
  }

  /**
   * Adds a click-sort to the query.
   *
   * @param $order
   *   Either 'ASC' or 'DESC'.
   */
  public static function click_sort($handler, $order) {
    // The normal orderby() method for this usually won't work here. So we need
    // query plugins to provide their own method for this.
    if (method_exists($handler->query, 'add_selector_orderby')) {
      $selector = self::construct_property_selector($handler, TRUE);
      $handler->query->add_selector_orderby($selector, $order);
    }
  }

  /**
   * Load the entities for all rows that are about to be displayed.
   *
   * Automatically takes care of relationships, either defined via Views or via
   * Entity API data selectors.
   */
  public static function pre_render($handler, &$values) {
    if (empty($values)) {
      return;
    }
    if (method_exists($handler->query, 'get_result_wrappers')) {
      list($type, $wrappers) = $handler->query->get_result_wrappers($values, NULL);
    }
    elseif (method_exists($handler->query, 'get_result_entities')) {
      list($type, $entities) = $handler->query->get_result_entities($values, NULL);
      $wrappers = entity_metadata_wrapper_multiple($type, $entities);
    }
    else {
      return;
    }

    $relationship = self::construct_property_selector($handler);
    if ($relationship) {
      $wrappers = self::extract_property_multiple($wrappers, $relationship);
      unset($entities);
      $w = $wrappers;
      while (is_array($w)) {
        $w = array_filter($w);
        $w = reset($w);
      }
      if ($w) {
        $type = $w->type();
      }
    }
    $wrappers = array_filter($wrappers);

    $handler->entity_type = $type;
    $handler->entities = array();
    $handler->wrappers = $wrappers;
    if (isset($entities)) {
      $handler->entities = array_filter($entities);
    }
    elseif (entity_get_info($type)) {
      // We can just load the entities from the wrappers.
      foreach ($wrappers as $id => $wrapper) {
        $handler->entities[$id] = $wrapper->value();
      }
    }
  }

  /**
   * Return a Entity API data selector for the given handler's relationship.
   *
   * @param $handler
   *   The handler for which to construct the selector.
   * @param $complete
   *   If TRUE, the complete selector for the field is returned, not just the
   *   one for its parent. Defaults to FALSE.
   *
   * @return
   *   An Entity API data selector for the given handler's relationship.
   *
   * @see extract_property_multiple()
   */
  public static function construct_property_selector($handler, $complete = FALSE) {
    $ret = '';
    if ($handler->relationship) {
      $h = $handler;
      $view = $h->view;
      while (!empty($h->relationship) && !empty($view->relationship[$h->relationship])) {
        $h = $view->relationship[$h->relationship];
        $ret = $h->real_field . ($ret ? ":$ret" : '');
      }
    }

    // If we have a selector as the real_field, append this to the returned
    // relationship selector.
    if ($complete) {
      $ret .= ($ret ? ':' : '') . $handler->real_field;
    }
    elseif ($pos = strrpos($handler->real_field, ':')) {
      $ret .= ($ret ? ':' : '') . substr($handler->real_field, 0, $pos);
    }

    return $ret;
  }

  /**
   * Overridden to use a metadata wrapper (if necessary).
   *
   * Uses $values->_entity_properties to look for already extracted fields.
   *
   * @param $handler
   *   The field handler for which to return a value.
   * @param $values
   *   The values for the current row retrieved from the Views query, as an
   *   object.
   * @param $field
   *   The field to extract. If no value is given, the field of the given
   *   handler is used instead. The special "entity object" value can be used to
   *   get the base entity instead of a special field.
   * @param $default
   *   The value to return if the entity or field are not present.
   */
  public static function get_value($handler, $values, $field = NULL, $default = NULL) {
    // There is a value cache on each handler so parent handlers rendering a
    // single field value from a list will get the right data.
    if (isset($handler->_cache[$field])) {
      return $handler->_cache[$field];
    }
    $field = isset($field) ? $field : $handler->base_field;
    $selector = self::construct_property_selector($handler);
    $selector = $selector ? "$selector:$field" : $field;
    if (!isset($values->_entity_properties[$selector])) {
      if (!isset($handler->wrappers[$handler->view->row_index])) {
        $values->_entity_properties[$selector] = $default;
      }
      else {
        $wrapper = $handler->wrappers[$handler->view->row_index];
        if ($field === 'entity object') {
          $values->_entity_properties[$selector] = $wrapper->value();
        }
        else {
          $values->_entity_properties[$selector] = isset($wrapper->$field) ? $wrapper->$field->value(array('identifier' => TRUE)) : $default;
        }
      }
    }
    return $values->_entity_properties[$selector];
  }

  /**
   * Render the field.
   *
   * Implements the entity link functionality and list handling. Basic handling
   * of the single values is delegated back to the field handler.
   *
   * @param $handler
   *   The field handler whose field should be rendered.
   * @param $values
   *   The values for the current row retrieved from the Views query, as an
   *   object.
   *
   * @return
   *   The rendered value for the field.
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
   *
   * @param $handler
   *   The field handler whose field is rendered.
   * @param $list
   *   The list of values to render.
   * @param $values
   *   The values for the current row retrieved from the Views query, as an
   *   object.
   *
   * @return
   *   The rendered value for the given list.
   */
  public static function render_list($handler, $list, $values) {
    // Allow easy overriding of this behaviour in the specific field handler.
    if (method_exists($handler, 'render_list')) {
      return $handler->render_list($list, $values);
    }
    if (isset($handler->options['list']['mode'])) {
      if ($handler->options['list']['mode'] == 'first') {
        $list = count($list) ? array_shift($list) : NULL;
        if (is_array($list)) {
          return self::render_entity_link($handler, $list, $values);
        }
        elseif (isset($list)) {
          return self::render_entity_link($handler, $list, $values);
        }
        return NULL;
      }
      if ($handler->options['list']['mode'] == 'count') {
        return count($list);
      }
    }
    $inner_values = array();
    foreach ($list as $value) {
      $value = is_array($value) ? self::render_list($handler, $value, $values) : self::render_entity_link($handler, $value, $values);
      if ($value) {
        $inner_values[] = $value;
      }
    }
    $glue = isset($handler->options['list']['glue']) ? $handler->options['list']['glue'] : ', ';
    return implode($glue, $inner_values);
  }

  /**
   * Render a single value as a link to the entity, if applicable.
   *
   * @param $handler
   *   The field handler whose field is rendered.
   * @param $value
   *   The single value to render.
   * @param $values
   *   The values for the current row retrieved from the Views query, as an
   *   object.
   *
   * @return
   *   The rendered value.
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
      return l($render, $url['path'], array('html' => TRUE) + $url['options']);
    }
    return $render;
  }

  /**
   * Render a single value.
   *
   * @param $handler
   *   The field handler whose field is rendered.
   * @param $value
   *   The single value to render.
   * @param $values
   *   The values for the current row retrieved from the Views query, as an
   *   object.
   *
   * @return
   *   The rendered value.
   */
  public static function render_single_value($handler, $value, $values) {
    // Try to use the method in the specific field handler.
    if (method_exists($handler, 'render_single_value')) {
      $handler->_cache[NULL] = $value;
      $ret = $handler->render_single_value($value, $values);
      unset($handler->_cache[NULL]);
      return $ret;
    }
    // Lame fallback in case the field handler doesn't provide the method.
    return is_scalar($value) ? check_plain($value) : nl2br(check_plain(print_r($value, TRUE)));
  }

  /**
   * Extracts data from several metadata wrappers based on a data selector.
   *
   * All metadata wrappers passed to this function have to be based on the exact
   * same property information. The data will be returned wrapped by one or more
   * metadata wrappers.
   *
   * A data selector is a concatenation of properties which should be followed
   * to arrive at a desired property, that may be nested in related entities or
   * structures. The separate properties are herein concatenated with colons (:).
   *
   * For instance, a data selector of "author:roles" would mean to first
   * access the "author" property of the given wrapper, and then for this new
   * wrapper to access and return the "roles" property.
   *
   * Lists of values in a property are automatically handled. They will result in
   * nested arrays being returned.
   *
   * @param array $wrappers
   *   The EntityMetadataWrapper objects from which to extract data.
   * @param $selector
   *   The selector specifying the data to extract.
   * @param array $options
   *   An array of options that should be passed to the
   *   EntityMetadataWrapper::value() method (see there).
   *
   * @return array
   *   The extracted property value(s) for each wrapper, keyed to the same key
   *   that was used for the respecive wrapper in $wrappers. All extracted
   *   properties are returned as metadata wrappers.
   */
  public static function extract_property_multiple(array $wrappers, $selector) {
    $parts = explode(':', $selector, 2);
    $name = $parts[0];

    $results = array();
    $entities = array();
    foreach ($wrappers as $i => $wrapper) {
      try {
        $property = $wrapper->$name;
        if ($property instanceof EntityDrupalWrapper) {
          $id = $property->getIdentifier();
          $entities[$property->type()][$id] = $i;
        }
        elseif ($property instanceof EntityStructureWrapper) {
          $results[$i] = $property;
        }
        elseif ($property instanceof EntityListWrapper) {
          foreach (EntityListWrapper as $item) {
            $results[$i][] = $item;
          }
        }
        // Do nothing in case it cannot be applied.
      }
      catch (EntityMetadataWrapperException $e) {
        // Skip single empty properties.
      }
    }

    if ($entities) {
      // Map back the loaded entities back to the results array.
      foreach ($entities as $type => $id_map) {
        $entities[$type] = entity_load($type, array_keys($id_map));
        foreach ($entities[$type] as $id => $entity) {
          $results[$id_map[$id]] = entity_metadata_wrapper($type, $entity);
        }
      }
    }

    // We need to handle multiple values.
    if (is_array(reset($results))) {
      $all_wrappers = array();
      $map = array();
      $j = 0;
      foreach ($results as $i => $item_wrappers) {
        foreach ($item_wrappers as $i => $item_wrapper) {
          $all_wrappers[$j] = $item_wrapper;
          $map[$j] = $i;
          $j++;
        }
      }
      $result = self::extract_property_multiple($all_wrappers, $selector);
      // Map the results back.
      foreach ($result as $j => $result_wrapper) {
        $results[$map[$j]][] = $result_wrapper;
      }
      return $results;
    }

    if (!empty($parts[1])) {
      return self::extract_property_multiple($results, $parts[1]);
    }
    return $results;
  }

}
