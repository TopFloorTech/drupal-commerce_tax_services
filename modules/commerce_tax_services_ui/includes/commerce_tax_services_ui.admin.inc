<?php

/**
 * @file
 * Admin settings for commerce_tax_services.
 *
 * Copyright (C) Alexander Bischoff, adTumbler.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

/**
 * Admin settings menu callback.
 * @param $form
 * @param $form_state
 * @return mixed
 */
function commerce_tax_services_ui_admin_settings($form, &$form_state) {
  include_once DRUPAL_ROOT . '/includes/locale.inc';

  $form['options'] = commerce_tax_services_ui_admin_options();

  $form['services'] = commerce_tax_services_ui_admin_services();

  $form['#validate'][] = 'commerce_tax_services_ui_validate_address_validation_settings';

  drupal_alter('tax_service_admin_options', $form, $form_state);

  return system_settings_form($form);
}

function commerce_tax_services_ui_admin_services() {
  $services = array(
    '#type' => 'container',
    '#prefix' => '<div id="commerce_tax_services_services">',
    '#suffix' => '</div>',
    '#tree' => FALSE,
  );

  $info = commerce_tax_services_info();
  $hook = 'tax_service_admin_options';
  foreach ($info as $service_id => $service_info) {
    if (!empty($service_info['module'])) {
      if (module_hook($service_info['module'], $hook)) {
        $services[$service_id] = array(
          '#type' => 'fieldset',
          '#title' => t('Service Settings: @name', array('@name' => $service_info['name'])),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#tree' => FALSE,
        );

        $services[$service_id]['options'] = module_invoke($service_info['module'], $hook);
      }
    }
  }

  return $services;
}

function commerce_tax_services_ui_admin_options() {
  $options = array(
    '#type' => 'container',
    '#prefix' => '<div id="commerce_tax_services_options">',
    '#suffix' => '</div>',
    '#tree' => FALSE,
  );

  // Configure sales tax description to be shown to users.
  $options['commerce_tax_services_tax_description'] = array(
    '#title' => t('Sales Tax Description'),
    '#description' => t('The Sales Tax description to be displayed on the order check out form'),
    '#type' => 'textfield',
    '#default_value' => variable_get('commerce_tax_services_tax_description', 'Sales tax'),
  );

  $options['shipping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Shipping settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => FALSE,
  );

  // Select the states that will use Tax Services to calculate the sales tax amount.
  $options['shipping']['commerce_tax_services_select_states'] = array(
    '#title' => t('Tax Services States'),
    '#description' => t('Select States - Leave blank for all states'),
    '#type' => 'select',
    '#multiple' => TRUE,
    '#options' => commerce_tax_services_states(),
    '#default_value' => variable_get('commerce_tax_services_select_states', array()),
  );


  // Configure address to use for sales tax.
  $options['shipping']['commerce_tax_services_tax_address'] = array(
    '#title' => t('Destination Address to Use for Sales Tax'),
    '#description' => t('Select the address type to use for sales tax calculation (usually Shipping)'),
    '#type' => 'select',
    '#options' => commerce_tax_services_address_options(),
    '#default_value' => variable_get('commerce_tax_services_tax_address', 'Shipping'),
    '#ajax' => array(
      'callback' => 'commerce_tax_services_ui_tax_address_ajax_callback',
      'wrapper' => 'address_validation_fieldset_wrapper',
    ),
  );

  // Set Street, City, State and Zip for Primary Business Office Location.
  $options['shipping']['commerce_tax_services_account_name'] = array(
    '#title' => t('Business or Company Name'),
    '#description' => t('Registered Name of Company or Business'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_tax_services_account_name', ''),
  );
  $options['shipping']['commerce_tax_services_primary_street1'] = array(
    '#title' => t('Primary Business Street 1'),
    '#description' => t('The Primary Street address your business is located at'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_tax_services_primary_street1', ''),
  );
  $options['shipping']['commerce_tax_services_primary_street2'] = array(
    '#title' => t('Primary Business Street 2'),
    '#description' => t('The Primary Street address your business is located at (continued)'),
    '#type' => 'textfield',
    '#default_value' => variable_get('commerce_tax_services_primary_street2', ''),
  );
  $options['shipping']['commerce_tax_services_primary_city'] = array(
    '#title' => t('Primary Business City'),
    '#description' => t('The Primary City your business is located in'),
    '#required' => TRUE,
    '#type' => 'textfield',
    '#default_value' => variable_get('commerce_tax_services_primary_city', ''),
  );
  $options['shipping']['commerce_tax_services_primary_state'] = array(
    '#title' => t('Primary Business State'),
    '#description' => t('The Primary State your business is located in'),
    '#type' => 'select',
    '#options' => commerce_tax_services_states(),
    '#default_value' => variable_get('commerce_tax_services_primary_state', ''),
  );
  $options['shipping']['commerce_tax_services_primary_country'] = array(
    '#title' => t('Primary Business Country'),
    '#description' => t('The primary country your business is located in'),
    '#type' => 'select',
    '#required' => TRUE,
    '#options' => country_get_list(),
    '#default_value' => variable_get('commerce_tax_services_primary_country', ''),
  );
  $options['shipping']['commerce_tax_services_primary_zip'] = array(
    '#title' => t('Primary Business Zip'),
    '#description' => t('The Primary Zip Code your business is located in. Must be a Valid 5 digit zip'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => variable_get('commerce_tax_services_primary_zip', ''),
  );

  $options['address_validate_wrapper'] = commerce_tax_services_ui_admin_address_validation_options($form, $form_state);
  
  return $options;
}

/**
 * @param $form
 * @param $form_state
 * @return array
 */
function commerce_tax_services_ui_admin_address_validation_options(&$form, &$form_state) {
  $options = array(
    '#type' => 'container',
    '#prefix' => '<div id="address_validation_fieldset_wrapper">',
    '#suffix' => '</div>',
  );

  $tax_address = (isset($form_state['values']['commerce_tax_services_tax_address']))
    ? $form_state['values']['commerce_tax_services_tax_address']
    : variable_get('commerce_tax_services_tax_address', 'Shipping');
  
  if ($tax_address != 'Shipping') {
    return $options;
  }

  $options['address_validate'] = array(
    '#type' => 'fieldset',
    '#title' => t('Address validation'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => FALSE,
  );

  $options['address_validate']['commerce_tax_services_validate_address'] = array(
    '#type' => 'checkbox',
    '#title' => t('Validate checkout shipping address'),
    '#default_value' => variable_get('commerce_tax_services_validate_address', TRUE),
    '#ajax' => array(
      'callback' => 'commerce_tax_services_ui_address_validate_ajax_callback',
      'wrapper' => 'commerce_tax_services_address_validate_options',
    ),
  );

  $options['address_validate']['config'] = array(
    '#type' => 'container',
    '#tree' => FALSE,
    '#prefix' => '<div id="commerce_tax_services_address_validate_options">',
    '#suffix' => '</div>',
  );

  $validate_address = (isset($form_state['values']['commerce_tax_services_validate_address']))
    ? $form_state['values']['commerce_tax_services_validate_address']
    : variable_get('commerce_tax_services_validate_address', TRUE);

  if (!$validate_address) {
    return $options;
  }

  $options['address_validate']['config']['commerce_tax_services_address_validate_countries'] = array(
    '#type' => 'select',
    '#title' => t('Countries'),
    '#description' => t('Countries to validate addresses?'),
    '#options' => array(
      'US' => t('United States'),
    ),
    '#default_value' => (array) variable_get('commerce_tax_services_address_validate_countries', array('US')),
    '#required' => TRUE,
  );

  $options['address_validate']['config']['commerce_tax_services_address_postal_code'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match on postal code'),
    '#description' => t('Postal codes are 9 digits, but most people enter the first 5 digits, do you want AvaTax to match all 9 digits?'),
    '#default_value' => variable_get('commerce_tax_services_address_postal_code', TRUE),
  );

  $options['address_validate']['config']['commerce_tax_services_autocomplete_postal_code'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto complete 5 digit postal code to 9 digits'),
    '#description' => t('Automatically insert the 9 digit postal code provided by AvaTax'),
    '#default_value' => variable_get('commerce_tax_services_autocomplete_postal_code', TRUE),
  );
  
  return $options;
}

/**
 * Disable address validation when address is taken from Billing profile.
 * @param $form
 * @param $form_state
 */
function commerce_tax_services_ui_validate_address_validation_settings($form, &$form_state) {
  if ($form_state['values']['commerce_tax_services_tax_address'] == 'Billing') {
    $form_state['values']['commerce_tax_services_validate_address'] = FALSE;
    variable_set('commerce_tax_services_validate_address', FALSE);
  }
}

/**
 * Returns address validation fieldset using ajax.
 * @param $form
 * @param $form_state
 * @return mixed
 */
function commerce_tax_services_ui_tax_address_ajax_callback($form, &$form_state) {
  return $form['options']['address_validate_wrapper'];
}

/**
 * If address validation is enabled, returns its config form items.
 * @param $form
 * @param $form_state
 * @return mixed
 */
function commerce_tax_services_ui_address_validate_ajax_callback($form, &$form_state) {
  return $form['options']['address_validate_wrapper']['address_validate']['config'];
}

/**
 * Ajax callback, returns updated shipping states form element.
 * @param $form
 * @param $form_state
 * @return mixed
 */
function commerce_tax_services_ui_update_form_options($form, &$form_state) {
  return $form['options'];
}
