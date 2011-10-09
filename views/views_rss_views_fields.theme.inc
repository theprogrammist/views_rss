<?php

/**
 * Template preprocessor for views-view-views-rss-fields.tpl.php.
 */
function template_preprocess_views_view_views_rss_fields(&$vars) {
  $view = $vars['view'];

  // Set basic info - title, description - about the feed
  if ($view->display_handler->get_option('sitename_title')) {
    $title = variable_get('site_name', 'Drupal');
    if ($slogan = variable_get('site_slogan', '')) {
      $title .= ' - ' . $slogan;
    }
  }
  else {
    $title = $view->get_title();
  }
  $vars['viewtitle'] = check_plain($title);

  if ($view->style_options['description']['feed_description']) {
    $description = $view->style_options['description']['feed_description'];
  }
  else {
    $description = variable_get('site_mission', '');
  }
  $vars['description'] = theme('views_rss_feed_description', array('0' => $description, '1' => $view));

  // Base URL for link tag
  global $base_url;
  $vars['link'] = $base_url;

  // Grab the rows, push to field mapping function, gather namespaces.
  $elements = $namespaces = array();
  $rows = '';
  $items = $view->style_plugin->map_rows($vars['rows']);
  foreach ($items as $item) {
    // Special handling for GeoRSS.
    if (isset($item['lat']) && is_numeric($item['lat']) && isset($item['lon']) && is_numeric($item['lon'])) {
      $item['georss:point'] = $item['lat'] . ' ' .  $item['lon'];
    }
    if (isset($item['featureName'])) {
      $item['georss:featureName'] = $item['featureName'];
    }
    unset($item['lat']);
    unset($item['lon']);
    unset($item['featureName']);
    $rows .= theme('views_rss_fields_item', array('item' => $item));
    foreach ($item as $k => $v) {
      $elements[$k] = $k;
    }
  }
  $vars['rows'] = $rows;
  foreach ($elements as $e) {
    if ($namespace = $view->style_plugin->xml_namespace($e)) {
      $namespaces[] = "xmlns:{$namespace['local']}=\"{$namespace['namespace']}\"";
    }
  }
  $vars['namespaces'] = implode(' ', array_unique($namespaces));

  // Set Headers
  // During live preview we don't want to output the header since the contents
  // of the feed are being displayed inside a normal HTML page.
  if (empty($vars['view']->live_preview)) {
    drupal_add_http_header('Content-Type', 'application/rss+xml; charset=utf-8');
  }
}

/**
 * Element template
 */
function theme_views_rss_fields_item($variables) {
  $item = $variables['item'];
  // Loop through key=>value pairs
  $row = '';
  foreach ($item as $key => $value) {
    if ($value) {
      $row .= theme('views_rss_fields_element', array('key' => $key, 'element' => $value));
    }
  }
  return '<item>' . $row . '</item>';
}

/**
 * Element template
 */
function theme_views_rss_fields_element($variables) {
  $key = $variables['key'];
  $value = $variables['element'];

  if ($key == 'enclosure') {
    // file exists
    if (isset($value['#raw'][0])) {
      $file = (array)$value['#raw'][0]['raw'];
      // Media field
      if (isset($value['#raw'][0]['rendered']['file']['#entity_type']) &&
          $value['#raw'][0]['rendered']['file']['#entity_type'] == 'media') {
        $style = str_replace("styles_file_", "", $value['#raw'][0]['rendered']['file']['#formatter']);
        $url = image_style_url($style, $file['uri']);
      }
      // Image field
      else if (isset($value['#raw'][0]['rendered']['#image_style'])) {
        $url = image_style_url($value['#raw'][0]['rendered']['#image_style'], $file['uri']);
      }
      // File field
      else {
        $url = file_create_url($file['uri']);
      }
      return '<enclosure url="' . $url . '" length="' . $file['filesize'] . '" type="' . $file['filemime'] . '" />';
    }
  }
  else {
    return "<$key>" . check_plain(htmlspecialchars_decode($value['#markup'])) . "</$key>";
  }
}

/**
 * Theme function for feed icon.
 */
function theme_views_rss_feed_icon($variables) {
  $url = $variables['0'];
  $title = $variables['1'];
  $icon = $variables['2'];
  if ($image = theme('image', array('path' => $icon, 'width' => t('Download RSS Feed'), 'height' => $title))) {
    return '<a href="' . check_url($url) . '" class="feed-icon">' . $image . '</a>';
  }
}

/**
 * Theme function for feed description.
 */
function theme_views_rss_feed_description($variables) {
  $description = $variables['0'];
  $view = $variables['1'];
  return $description;
}
