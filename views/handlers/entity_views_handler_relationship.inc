<?php

/**
 * @file
 * Contains the entity_views_handler_relationship class.
 */

/**
 * Relationship handler for pure entity tables.
 *
 * @ingroup views_field_handlers
 */
class entity_views_handler_relationship extends views_handler_relationship {

  /**
   * Called to implement a relationship in a query.
   *
   * As we don't add any data to the query itself, we don't have to do anything
   * here. Views just don't thinks we have been called unless we set our
   * $alias property. Otherwise, this override is just here to keep PHP from
   * blowing up by calling inexistent methods on the query plugin.
   */
  public function query() {
    $this->alias = $this->real_field;
  }

}
