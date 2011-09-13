<?php

/**
 * @file
 * Provide views data for modules making use of the entity CRUD API.
 */

/**
 * Implements hook_views_data()
 */
function entity_views_data() {
  $data = array();

  foreach (entity_crud_get_info() as $type => $info) {
    // Only enable the integration by default if we know the module providing
    // the entity and it does not provide views integration yet.
    if (!isset($info['views controller class'])) {
      $info['views controller class'] = isset($info['module']) && !module_hook($info['module'], 'views_data') ? 'EntityDefaultViewsController' : FALSE;
    }
    if ($info['views controller class']) {
      $controller = new $info['views controller class']($type);
      // Relationship data may return views data for already existing tables,
      // so merge results on the second level.
      foreach ($controller->views_data() as $table => $table_data) {
        $data += array($table => array());
        $data[$table] = array_merge($data[$table], $table_data);
      }
    }
  }

  // Add generic (non-base) tables for all entity types.
  foreach (entity_get_info() as $type => $info) {
    $table = entity_views_table_definition($type);
    if ($table) {
      $data['entity_' . $type] = $table;
    }
  }

  return $data;
}

/**
 * Helper function for getting the entity Views table definition for a type.
 *
 * @param $type
 *   The entity type whose table definition should be return.
 *
 * @return
 *   An array containing a generic Views table definition for the entity type.
 */
function entity_views_table_definition($type) {
  $tables = &drupal_static(__FUNCTION__, array());

  if (!isset($tables[$type])) {
    $info = entity_get_info($type);
    $tables[$type]['table'] = array(
      'group' => $info['label'],
      'entity type' => $type,
    );
    foreach (entity_get_all_property_info($type) as $key => $property) {
      entity_views_field_definition($key, $property, $tables[$type]);
    }
  }

  return $tables[$type];
}

/**
 * Helper function for creating a Views field definition.
 *
 * @param $field
 *   The key to be used for the property's field handler (and as the prefix for
 *   nested ones).
 * @param array $property
 *   The property information for which to create a field definition.
 * @param array $table
 *   The table into which the definition should be inserted.
 * @param $title_prefix
 *   Internal use only.
 */
function entity_views_field_definition($field, array $property, array &$table, $title_prefix = '') {
  $additional = array();
  $additional_field = array();

  // Create a valid Views field identifier (no colons, etc.).
  $key = _entity_views_field_identifier($field, $table);
  if ($key != $field) {
    $additional['real field'] = $field;
  }

  // Field handlers for the entity tables, by type.
  $field_handlers = array(
    'text'         => 'entity_views_handler_field_text',
    'token'        => 'entity_views_handler_field_text',
    'integer'      => 'entity_views_handler_field_numeric',
    'decimal'      => 'entity_views_handler_field_numeric',
    'date'         => 'entity_views_handler_field_date',
    'duration'     => 'entity_views_handler_field_duration',
    'boolean'      => 'entity_views_handler_field_boolean',
    'uri'          => 'entity_views_handler_field_uri',
    'options'      => 'entity_views_handler_field_options',
    'field'        => 'entity_views_handler_field_field',
    'entity'       => 'entity_views_handler_field_entity',
    'relationship' => 'entity_views_handler_relationship',
  );
  // @todo Some alter hook?

  $property += entity_property_info_defaults();
  $type = entity_property_list_inner_type($property['type']);
  $title = $title_prefix . $property['label'];
  if (entity_get_info($type)) {
    $additional_field['entity type'] = $type;
    // Others might have unset the relationship handler in the alter hook.
    if (isset($field_handlers['relationship'])) {
      $additional['relationship'] = array(
        'handler' => $field_handlers['relationship'],
        'base' => 'entity_' . $type,
        'base field' => $key,
        'relationship field' => $field,
        'label' => $title,
      );
    }
    // They could also add handlers for specific entity types.
    if (!isset($field_handlers[$type])) {
      $type = 'entity';
    }
  }
  elseif (!empty($property['field'])) {
    $type = 'field';
    $field_name = ltrim(substr($key, strrpos($key, ':')), ':');
    $additional_field['field_name'] = $field_name;
    $additional_field['entity_tables'] = array();
    $additional_field['entity type'] = $table['table']['entity type'];
    $additional_field['is revision'] = FALSE;
  }
  // Copied from EntityMetadataWrapper::optionsList()
  elseif (isset($property['options list']) && is_callable($property['options list'])) {
    // If this is a nested property, we need to get rid of all prefixes first.
    $field_name = ltrim(substr($key, strrpos($key, ':')), ':');
    $options = call_user_func($property['options list'], $field_name, $property, 'view');
    if ($options) {
      $type = 'options';
      $additional_field['options'] = $options;
    }
  }
  elseif ($type == 'decimal') {
    $additional_field['float'] = TRUE;
  }

  if (isset($field_handlers[$type])) {
    $table += array($key => array());
    $table[$key] += array(
      'title' => $title,
      'help' => empty($property['description']) ? t('(No information available)') : $property['description'],
      'field' => array(),
    );
    $table[$key]['field'] += array(
      'handler' => $field_handlers[$type],
      'type' => $property['type'],
    );
    $table[$key]['field'] += $additional_field;
    $table[$key] += $additional;
  }
  if (!empty($property['property info'])) {
    foreach ($property['property info'] as $nested_key => $nested_property) {
      entity_views_field_definition($key . ':' . $nested_key, $nested_property, $table, $title . ' » ');
    }
  }
}

/**
 * Helper function for creating valid Views field identifiers out of property accessors.
 *
 * @return string
 *   A valid Views field identifier that isn't yet used as a key in $table.
 */
function _entity_views_field_identifier($field, array $table) {
  $aliases = &drupal_static(__FUNCTION__, array());
  // If we re-use aliases (tables might be different), we have to make sure we
  // never return the same alias for two different fields (even this is highly
  // unlikely).
  $used = array_flip($aliases);

  if (!isset($aliases[$field])) {
    $key = $base = preg_replace('/[^a-zA-Z0-9]+/S', '_', $field);
    $i = 0;
    while (isset($table[$key]) || (isset($used[$key]) && $used[$key] !== $field)) {
      $key = $base . '_' . ++$i;
    }
    $aliases[$field] = $key;
  }

  return $aliases[$field];
}

/**
 * Implements hook_views_plugins().
 */
function entity_views_plugins() {
  foreach (views_fetch_data() as $table => $data) {
    if (!empty($data['table']['base']['entity type'])) {
      $base_tables[] = $table;
    }
  }
  if (!empty($base_tables)) {
    return array(
      'row' => array(
        'entity' => array(
          'title' => t('Entity'),
          'help' => t('Displays a single entity in a specific view mode (e.g. teaser).'),
          'handler' => 'entity_plugin_row_entity_view',
          'uses fields' => FALSE,
          'uses options' => TRUE,
          'type' => 'normal',
          'base' => $base_tables,
        ),
      ),
    );
  }
}

/**
 * Default controller for generating basic views integration.
 *
 * The controller tries to generate suiting views integration for the entity
 * based upon the schema information of its base table and the provided entity
 * property information.
 * For that it is possible to map a property name to its schema/views field
 * name by adding a 'schema field' key with the name of the field as value to
 * the property info.
 */
class EntityDefaultViewsController {

  protected $type, $info, $relationships;

  public function __construct($type) {
    $this->type = $type;
    $this->info = entity_get_info($type);
  }

  /**
   * Defines the result for hook_views_data().
   */
  public function views_data() {
    $data = array();
    $this->relationships = array();

    if (!empty($this->info['base table'])) {
      $table = $this->info['base table'];
      // Define the base group of this table. Fields that don't
      // have a group defined will go into this field by default.
      $data[$table]['table']['group']  = drupal_ucfirst($this->info['label']);

      $data[$table]['table']['base'] = array(
        'field' => $this->info['entity keys']['id'],
        'title' => drupal_ucfirst($this->info['label']),
        // @todo: Support an entity info description key or such?
        'help' => '',
        'entity type' => $this->type,
      );
      $data[$table] += $this->schema_fields();

      // Add in any reverse-relationships which have been determined.
      $data += $this->relationships;
    }
    return $data;
  }

  /**
   * Try to come up with some views fields with the help of the schema and
   * the entity property information.
   */
  protected function schema_fields() {
    $schema = drupal_get_schema($this->info['base table']);
    $properties = entity_get_property_info($this->type) + array('properties' => array());
    $data = array();

    foreach ($properties['properties'] as $name => $property_info) {
      if (isset($property_info['schema field']) && isset($schema['fields'][$property_info['schema field']])) {
        if ($views_info = $this->map_from_schema_info($name, $schema['fields'][$property_info['schema field']], $property_info)) {
          $data[$name] = $views_info;
        }
      }
    }
    return $data;
  }

  /**
   * Comes up with views information based on the given schema and property
   * info.
   */
  protected function map_from_schema_info($property_name, $schema_field_info, $property_info) {
    $type = isset($property_info['type']) ? $property_info['type'] : 'text';
    $views_field_name = $property_info['schema field'];

    $return = array();

    if (!empty($schema_field_info['serialize'])) {
      return FALSE;
    }

    $description = array(
      'title' => $property_info['label'],
      'help' => $property_info['description'],
    );

      // Add in relationships to related entities.
    if (($info = entity_get_info($type)) && !empty($info['base table'])) {

      // Prepare reversed relationship data.
      $label_lowercase = drupal_strtolower($this->info['label'][0]) . drupal_substr($this->info['label'], 1);
      $property_label_lowercase = drupal_strtolower($property_info['label'][0]) . drupal_substr($property_info['label'], 1);

      $this->relationships[$info['base table']][$this->info['base table']] = array(
        'title' => $this->info['label'],
        'help' => t("Associated @label via the @label's @property.", array('@label' => $label_lowercase, '@property' => $property_label_lowercase)),
        'relationship' => array(
          'label' => $this->info['label'],
          'handler' => 'views_handler_relationship',
          'base' => $this->info['base table'],
          'base field' => $views_field_name,
          'relationship field' => isset($info['entity keys']['name']) ? $info['entity keys']['name'] : $info['entity keys']['id'],
        ),
      );

      $return['relationship'] = array(
        'label' => drupal_ucfirst($info['label']),
        'handler' => 'views_handler_relationship',
        'base' => $info['base table'],
        'base field' => isset($info['entity keys']['name']) ? $info['entity keys']['name'] : $info['entity keys']['id'],
        'relationship field' => $views_field_name,
      );

      // Add in direct field/filters/sorts for the id itself too.
      $type = isset($info['entity keys']['name']) ? 'token' : 'integer';
      // Append the views-field-name to the title if it is different to the
      // property name.
      if ($property_name != $views_field_name) {
        $description['title'] .= ' ' . $views_field_name;
      }
    }

    switch ($type) {
      case 'token':
      case 'text':
        $return += $description + array(
          'field' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_field',
            'click sortable' => TRUE,
           ),
          'sort' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_sort',
          ),
          'filter' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_filter_string',
          ),
          'argument' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_argument_string',
          ),
        );
      break;

      case 'decimal':
      case 'integer':
        $return += $description + array(
          'field' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_field_numeric',
            'click sortable' => TRUE,
            'float' => ($type == 'decimal'),
           ),
          'sort' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_sort',
          ),
          'filter' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_filter_numeric',
          ),
          'argument' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_argument_numeric',
          ),
        );
      break;

      case 'date':
        $return += $description + array(
          'field' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_field_date',
            'click sortable' => TRUE,
           ),
          'sort' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_sort_date',
          ),
          'filter' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_filter_date',
          ),
          'argument' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_argument_date',
          ),
        );
      break;

      case 'uri':
        $return += $description + array(
          'field' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_field_url',
            'click sortable' => TRUE,
           ),
          'sort' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_sort',
          ),
          'filter' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_filter_string',
          ),
          'argument' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_argument_string',
          ),
        );
      break;

      case 'boolean':
        $return += $description + array(
          'field' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_field_boolean',
            'click sortable' => TRUE,
           ),
          'sort' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_sort',
          ),
          'filter' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_filter_boolean_operator',
          ),
          'argument' => array(
            'real field' => $views_field_name,
            'handler' => 'views_handler_argument_string',
          ),
        );
      break;
    }

    // If there is an options list callback, add to the filter.
    if (isset($return['filter']) && !empty($property_info['options list'])) {
      $return['filter']['handler'] = 'views_handler_filter_in_operator';
      $return['filter']['options callback'] = array('EntityDefaultViewsController', 'optionsListCallback');
      $return['filter']['options arguments'] = array($this->type, $property_name, 'view');
    }
    return $return;
  }

  /**
   * A callback returning property options, suitable to be used as views options callback.
   */
  public static function optionsListCallback($type, $selector, $op = 'view') {
    $wrapper = entity_metadata_wrapper($type, NULL);
    $parts = explode(':', $selector);
    foreach ($parts as $part) {
      $wrapper = $wrapper->get($part);
    }
    return $wrapper->optionsList($op);
  }
}
