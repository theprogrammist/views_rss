<?php

/**
 * @file
 * Preprocess functions for Views RSS: Core Elements module.
 */

/**
 * Preprocess function for channel <title> element.
 */
function views_rss_core_preprocess_channel_title($vars) {
  $view_title = $vars['view']->get_title();
  return ($view_title) ? $view_title : variable_get('sitename', t('Drupal'));
}

/**
 * Preprocess function for channel <description> element.
 */
function views_rss_core_preprocess_channel_description($vars) {
  $description = ($vars['value']) ? $vars['value'] : variable_get('site_mission', '');
  return check_plain(htmlspecialchars_decode($description, ENT_QUOTES));
}

/**
 * Preprocess function for channel <link> element.
 */
function views_rss_core_preprocess_channel_link($vars) {
  global $base_url;
  return $base_url;
}

/**
 * Preprocess function for item <pubDate> and <lastBuildDate> elements.
 *
 * It will return values for date element providing that original Views query
 * was modified appropriately by views_rss_core_views_query_alter() by adding
 * new fields to SELECT clause retrieving object creation (for <pubDate>)
 * or modification timestamp (for <lastBuildDate>).
 */
function views_rss_core_preprocess_channel_date($vars) {
  if (count($vars['view']->result) > 0) {
    $max_date = 0;
    foreach ($vars['view']->result as $row) {
      if (isset($row->{strtolower($vars['element'])}) && $row->{strtolower($vars['element'])} > $max_date) {
        $max_date = $row->{strtolower($vars['element'])};
      }
    }
    if ($max_date) {
      return date('r', $max_date);
    }
  }
}

/**
 * Preprocess function for item <description> element.
 */
function views_rss_core_preprocess_item_description($vars) {
  return check_plain(htmlspecialchars_decode($vars['value'], ENT_QUOTES));
}

/**
 * Preprocess function for item <guid> element.
 */
function views_rss_core_preprocess_item_guid($vars) {
  if ($vars['value']) {
    $value = array(
      'value' => $vars['value'],
      'arguments' => array(
        'isPermaLink' => 'false',
      ),
    );
    if ((!isset($vars['item']['link']) || empty($vars['item']['link'])) && valid_url($value['value'], TRUE)) {
      $value['arguments']['isPermaLink'] = 'true';
    }
    return $value;
  }
}

/**
 * Preprocess function for channel <source> element.
 */
function views_rss_core_preprocess_item_source($vars) {
  global $base_url;
  $value = array(
    'value' => $vars['view']->get_title(),
    'arguments' => array(
      'url' => $base_url . '/' . $vars['view']->get_url(),
    ),
  );
  // Add exposed filter values if enabled.
  $exposed_input = $vars['view']->get_exposed_input();
  if ($vars['view']->style_plugin->options['feed_settings']['exposed_in_source'] && $exposed_input) {
    $value['arguments']['url'] .= '?' . http_build_query($exposed_input, '', '&');
  }
  return $value;
}

/**
 * Theme function for channel <cloud> element.
 */
function views_rss_core_preprocess_item_cloud($vars) {
  if ($vars['value']) {
    if ($url = parse_url($vars['value'])) {
      return array(
        'arguments' => array(
          'domain' => $url['host'],
          'port' => $url['port'],
          'path' => $url['path'],
          'registerProcedure' => $url['fragment'],
          'protocol' => $url['scheme'],
        ),
      );
    }
  }
}