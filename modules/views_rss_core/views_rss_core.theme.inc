<?php

/**
 * @file
 * Theme functions for Views RSS: Core Elements module.
 */

/**
 * Theme function for channel <category> element.
 */
function theme_views_rss_core_element_category($vars) {
  $categories = array_map('trim', explode(',', $vars['value']));
  $output = '';
  foreach ($categories as $category) {
    $output .= theme('views_rss_element', array(
      'element' => $vars['element'],
      'value' => $category,
    ));
  }
  return $output;
}

/**
 * Theme function for channel <skipHours> and <skipDays> elements.
 */
function theme_views_rss_core_element_skip($vars) {
  $values = array_map('trim', explode(',', strip_tags($vars['value'])));
  switch ($vars['element']) {
    case 'skipHours':
      $subelement = 'hour';
      break;
    case 'skipDays':
      $subelement = 'day';
      break;
  }
  $output = "\n";
  foreach ($values as $value) {
    $output .= theme('views_rss_element', array(
      'element' => $subelement,
      'value' => $value,
    ));
  }
  return theme('views_rss_element', array(
    'element' => $vars['element'],
    'value' => $output,
  ));
}

/**
 * Formatter for 'image' and 'filefield' CCK fields containing images
 * added to <enclosure> element. Besides default image size
 * also provides option to use imagecache'd images.
 */
function theme_views_rss_core_formatter_enclosure($variables) {
  // Inside a view item may contain NULL data. In that case, just return.
  if (empty($variables['item']['fid'])) {
    return;
  }
  // Get full image URL based on provided image style.
  if ($variables['field']['type'] == 'image' && !empty($variables['image_style'])) {
    $path = image_style_url($variables['image_style'], $variables['item']['uri']);
  }
  else {
    $path = file_create_url($variables['item']['uri']);
  }
  // Return formatted XML element.
  return strtr('<enclosure url="!url" length="!length" type="!type" />', array(
    '!url'    => $path,
    '!length' => $variables['item']['filesize'],
    '!type'   => $variables['item']['filemime'],
  )) . "\n";
}
