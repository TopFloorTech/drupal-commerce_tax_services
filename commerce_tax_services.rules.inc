<?php

/**
 * @file
 * Rules supporting AvaTax Sales Order Processing.
 */

/**
 * Implements hook_rules_action_info().
 */
function commerce_tax_services_rules_action_info() {
  $default = array(
    'group' => t('Commerce Tax Services'),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Commerce Order'),
      ),
    ),
  );

  $actions = array(
    'commerce_tax_services_calculate_sales_tax' => 'Calculate sales tax for order',
    'commerce_tax_services_rules_delete_tax_service_line_items' => 'Delete tax service line items',
    'commerce_tax_services_rules_commit_transaction' => 'Change status of sales tax to COMMITTED in tax service',
    'commerce_tax_services_rules_cancel_transaction' => 'Change status of sales tax to VOIDED in tax service',
  );

  foreach ($actions as $action => $label) {
    $actions[$action] = array('label' => t($label)) + $default;
  }

  return $actions;
}

/**
 * DELETE Tax Services line item for $order.
 * @param $order
 */
function commerce_tax_services_rules_delete_tax_service_line_items($order) {
  if (commerce_tax_services_check_address($order)) {
    commerce_tax_services_delete_tax_service_transaction($order);
  }
}

/**
 * COMMIT Tax Services transaction for $order.
 * @param $order
 */
function commerce_tax_services_rules_commit_transaction($order) {
  if (commerce_tax_services_check_address($order)) {
    commerce_tax_services_retrieve_sales_tax($order, TRUE);
  }
}

/**
 * VOID Tax Services transaction for $order.
 * @param $order
 */
function commerce_tax_services_rules_cancel_transaction($order) {
  if (commerce_tax_services_check_address($order)) {
    commerce_tax_services_cancel_transaction($order);
  }
}
