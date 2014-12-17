<?php

/**
 * @file
 * Preprocess functions for Views RSS: Core Elements module.
 */

use Drupal\Component\Utility\String;
use Drupal\Component\Utility\UrlHelper;
use Drupal\taxonomy\Entity\Term;

/**
 * Preprocess function for channel <title> element.
 */
function views_rss_core_preprocess_channel_title(&$variables) {
  $config = \Drupal::config('system.site');

  if ($variables['view']->display_handler->getOption('sitename_title')) {
    $title = $config->get('name');
    if ($slogan = $config->get('slogan')) {
      $title .= ' - ' . $slogan;
    }
  }
  else {
    $title = $variables['view']->getTitle();
  }

  $variables['elements'][0]['value'] = String::checkPlain($title);
}

/**
 * Preprocess function for channel <description> element.
 */
function views_rss_core_preprocess_channel_description(&$variables) {
  if (empty($variables['elements'][0]['value'])) {
    $variables['elements'][0]['value'] = variable_get('site_slogan', '');
  }
}

/**
 * Preprocess function for channel <link> element.
 */
function views_rss_core_preprocess_channel_link(&$variables) {
  $url_options = array('absolute' => TRUE);
  $input = $variables['view']->getExposedInput();
  if ($input) {
    $url_options['query'] = $input;
  }

  $variables['elements'][0]['value'] = _url($variables['view']->getUrl(), $url_options);
}

/**
 * Preprocess function for channel <atom:link> element.
 */
function views_rss_core_preprocess_channel_atom_link(&$variables) {
  $url_options = array('absolute' => TRUE);
  $input = $variables['view']->getExposedInput();
  if ($input) {
    $url_options['query'] = $input;
  }

  $variables['elements'][0]['attributes'] = array(
    'rel' => 'self',
    'href' => _url($variables['view']->getUrl(), $url_options),
  );
}

/**
 * Preprocess function for channel <language> element.
 */
function views_rss_core_preprocess_channel_language(&$variables) {
  global $language;
  if (empty($variables['elements'][0]['value'])) {
    $variables['elements'][0]['value'] = $language->language;
  }
}

/**
 * Preprocess function for channel <category> element.
 */
function views_rss_core_preprocess_channel_category(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }
  $elements = array();
  $categories = explode(',', $variables['elements'][0]['value']);
  foreach ($categories as $category) {
    $elements[] = array(
      'key' => 'category',
      'value' => trim($category),
    );
  }
  $variables['elements'] = $elements;
}

/**
 * Preprocess function for channel <image> element.
 */
function views_rss_core_preprocess_channel_image(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }
  $path = $variables['elements'][0]['value'];
  // Get value of channel <title> element from its preprocess function.
  views_rss_core_preprocess_channel_title($variables);
  $title = $variables['elements'][0]['value'];
  // Get value of channel <title> element from its preprocess function.
  views_rss_core_preprocess_channel_link($variables);
  $link = $variables['elements'][0]['value'];
  // Create subelements array.
  $variables['elements'][0]['value'] = array(
    'url' => file_create_url($path),
    'title' => $title,
    'link' => $link,
  );
  // Try to get image description from website's mission.
  $site_slogan = variable_get('site_slogan', '');
  if (!empty($site_slogan)) {
    $variables['elements'][0]['value']['description'] = $site_slogan;
  }
  // Get image width and height.
  $image = image_load($path);
  if (!empty($image)) {
    $variables['elements'][0]['value']['width'] = $image->info['width'];
    $variables['elements'][0]['value']['height'] = $image->info['height'];
  }
}

/**
 * Preprocess function for channel <pubDate> and <lastBuildDate> elements.
 *
 * It will return values for date element providing that original Views query
 * was modified appropriately by views_rss_core_views_query_alter() by adding
 * new fields to SELECT clause retrieving object creation (for <pubDate>)
 * or modification timestamp (for <lastBuildDate>).
 */
function views_rss_core_preprocess_channel_date(&$variables) {
  if (count($variables['view']->result) > 0) {
    $max_date = 0;
    foreach ($variables['view']->result as $row) {
      $key = $variables['elements'][0]['key'];
      if (isset($row->$key) && $row->$key > $max_date) {
        $max_date = $row->$key;
      }
    }
    if ($max_date) {
      $variables['elements'][0]['value'] = date('r', $max_date);
    }
  }
}

/**
 * Preprocess function for channel <skipHours> and <skipDays> elements.
 */
function views_rss_core_preprocess_channel_skip(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }
  $elements = array();
  $skips = strip_tags($variables['elements'][0]['value']);
  if (!empty($skips)) {
    foreach (explode(',', $skips) as $skip_value) {
      $elements[] = array(
        'key' => ($variables['elements'][0]['key'] == 'skipHours') ? 'hour' : 'day',
        'value' => trim($skip_value),
      );
    }
  }
  $variables['elements'][0]['value'] = $elements;
}

/**
 * Preprocess function for channel <cloud> element.
 */
function views_rss_core_preprocess_channel_cloud(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }
  if ($url = parse_url($variables['elements'][0]['value'])) {
    $variables['elements'][0]['value'] = NULL;
    $variables['elements'][0]['attributes'] = array(
      'domain' => $url['host'],
      'port' => $url['port'],
      'path' => $url['path'],
      'registerProcedure' => $url['fragment'],
      'protocol' => $url['scheme'],
    );
  }
}

/**
 * Preprocess function for item <category> element.
 *
 * See http://www.rssboard.org/rss-profile#element-channel-item-category
 * for RSS Advisory Board requirements/recommendations.
 *
 * @see RSSCategoryFormatter::viewElements()
 */
function views_rss_core_preprocess_item_category(&$variables) {
  // No raw values = no preprocessing.
  if (empty($variables['raw']['items'])) {
    return;
  }

  $variables['elements'] = array();
  foreach ($variables['raw']['items'] as $item) {
    // When 'Content: <vocabulary_name>' was selected for source of category
    // element, we will get all we need in raw values.
    if ($item['raw']->target_id) {
      $term = $item['raw']->entity;
    }
    // On the other hand, when 'Content: All taxonomy terms' field was added
    // as a source for category element, we won't have raw values, instead
    // an array containing only tid, name, vocabulary_vid and vocabulary keys,
    // so we need to load full term object first before processing it further.
    elseif (!empty($item['tid'])) {
      $term = Term::load($item['tid']);
    }

    // According to RSS Advisory Board, the category's value should be
    // a slash-delimited string that identifies a hierarchical position
    // in the taxonomy.
    $values = array();
    // Load parent term objects (this includes original term as well).
    if ($parents = \Drupal::entityManager()->getStorage('taxonomy_term')->loadAllParents($term->id())) {
      foreach ($parents as $parent) {
        $values[] = $parent->label();
      }
    }

    $element = array(
      'key' => 'category',
      'value' => implode('/', array_reverse($values)),
      // Drupal uses term URL for domain attribute. RSS Best Practices say that
      // a domain attribute identifies the category's taxonomy - which suggests
      // either vocabulary name or its URL. We don't have any safe way to know
      // public vocabulary URL, could use its name instead though? @TODO?
      'attributes' => array(
        'domain' => $term->url('canonical', array('absolute' => TRUE)),
      ),
    );

    $variables['elements'][] = $element;
  }
}

/**
 * Preprocess function for item <enclosure> element.
 */
function views_rss_core_preprocess_item_enclosure(&$variables) {
  // No raw values = no preprocessing.
  if (empty($variables['raw']['items'])) {
    return;
  }

  $variables['elements'] = array();
  foreach ($variables['raw']['items'] as $item) {
    // File fields.
    if (!empty($item['rendered']['#file'])) {
      $file = $item['rendered']['#file'];
    }
    // Image fields.
    if (!empty($item['rendered']['#item']->entity)) {
      $file = $item['rendered']['#item']->entity;
    }

    // Start building RSS element.
    $element = array(
      'key' => 'enclosure',
      'attributes' => array(),
    );

    // File entity found.
    if (!empty($file)) {
      // Image style is defined, need to link to resized version.
      if ($image_style_name = $item['rendered']['#image_style']) {
        $image_style = entity_load('image_style', $image_style_name);
        $uri = $image_style->buildUri($file->getFileUri());
        $element['attributes'] = array(
          'url' => $uri,
          'length' => filesize(drupal_realpath($uri)),
          'type' => $file->getMimeType(),
        );
      }
      // Normal image size.
      else {
        $element['attributes'] = array(
          'url' => $file->url(),
          'length' => $file->getSize(),
          'type' => $file->getMimeType(),
        );
      }
    }
    // No file entity found, but something still was assigned to be displayed
    // as enclosure, so we just put its value in the url attribute.
    elseif (!empty($item['rendered']['#markup'])) {
      $element['attributes']['url'] = $item['rendered']['#markup'];
      $headers = get_headers($item['rendered']['#markup'], $format = 1);
      if (!empty($headers['Content-Length'])) {
        $element['attributes']['length'] = $headers['Content-Length'];
      }
      if (!empty($headers['Content-Type'])) {
        $element['attributes']['type'] = $headers['Content-Type'];
      }
    }

    $variables['elements'][] = $element;
  }
}

/**
 * Preprocess function for item <guid> element.
 */
function views_rss_core_preprocess_item_guid(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }
  $is_permalink = 'false';
  if (
    !empty($variables['item']['views_rss_core']['link'])
    && UrlHelper::isValid($variables['elements'][0]['value'], TRUE)
  ) {
    $is_permalink = 'true';
  }
  $variables['elements'][0]['attributes']['isPermaLink'] = $is_permalink;
}

/**
 * Preprocess function for item <source> element.
 */
function views_rss_core_preprocess_item_source(&$variables) {
  $config = \Drupal::config('system.site');

  if ($variables['view']->display_handler->getOption('sitename_title')) {
    $title = $config->get('name');
    if ($slogan = $config->get('slogan')) {
      $title .= ' - ' . $slogan;
    }
  }
  else {
    $title = $variables['view']->getTitle();
  }

  $url_options = array('absolute' => TRUE);
  $input = $variables['view']->getExposedInput();
  if ($input) {
    $url_options['query'] = $input;
  }

  $variables['elements'][0]['value'] = String::checkPlain($title);
  $variables['elements'][0]['attributes'] = array('url' => _url($variables['view']->getUrl(), $url_options));
}
