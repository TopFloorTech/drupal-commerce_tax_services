/**
 * @file
 * Handles AJAX submission for Tax Services address validation.
 */

(function ($) {
  function commerce_tax_services_attach_events() {
    $('input.checkout-continue').unbind('click').click(function (event) {
      if ($('form.commerce_checkout_form').hasClass('commerce_tax_services_revalidate_address')) {
        if (Drupal.settings.commerce_tax_services.commerce_tax_services_address_do_country_validate
          && Drupal.settings.commerce_tax_services.commerce_tax_services_address_validate_countries) {
          var enabled_countries = Drupal.settings.commerce_tax_services.commerce_tax_services_address_validate_countries;
          var selected_country = $('fieldset.' + Drupal.settings.commerce_tax_services.commerce_tax_services_address_validation_profile + ' select.country').val();

          if ($.inArray(selected_country, enabled_countries) < 0) {
            return true;
          }
        }

        event.preventDefault();

        $('input.address-validate-btn').trigger('mousedown');

        return false;
      }

      return true;
    });
  }

  function commerce_tax_services_clone_and_hide(element) {
    var $element = $(element);

    $element.clone().insertAfter($element).attr('disabled', true).next().removeClass('element-invisible');
    $element.hide();
  }

  function commerce_tax_services_remove_clone(element) {
    var $element = $(element);

    $($element[0]).show().attr('disabled', false);
    $($element[1]).remove();
  }

  function commerce_tax_services_handle_errors(errors) {
    for (var index in errors) {
      var parts = index.split('][');
      var name = parts[0] + "[" + parts.slice(1).join("][") + "]";

      $('input[name="' + name + '"]')
        .addClass('error')
        .parent()
        .append('<div class="inline error message">' + errors[index] + '</div>');
    }
  }

  Drupal.behaviors.taxServicesAddressValidate = {
    attach: function (context, settings) {
      $('form.commerce_checkout_form').addClass('commerce_tax_services_revalidate_address');

      commerce_tax_services_attach_events();
    }
  };

  Drupal.ajax.prototype.commands.afterAddressValidation = function (ajax, response, status) {
    $('.error.message').remove();

    $('.error').removeClass('error');

    var $continue_button = $('input.checkout-continue');
    var $processed_buttons = $continue_button.hasClass('processed');
    var $checkout_processing = $('span.checkout-processing');
    var $form = $('form.commerce_checkout_form');

    if (response.errors || response.validation_result === false) {
      commerce_tax_services_remove_clone($processed_buttons);

      $checkout_processing.addClass('element-invisible');

      commerce_tax_services_handle_errors(response.errors);

      return;
    }

    var validation_profile = Drupal.settings.commerce_tax_services.commerce_tax_services_address_validation_profile;

    if (response.validation_result.result == 'valid') {
      if (Drupal.settings.commerce_tax_services.commerce_tax_services_autocomplete_postal_code) {
        var address = response.validation_result.suggestions[0];

        $('fieldset.' + validation_profile + ' input.postal-code').val(address.postal_code);
      }

      $form.submit();

      return;
    }

    commerce_tax_services_remove_clone($processed_buttons);

    $checkout_processing.addClass('element-invisible');

    var buttons = [];

    if (response.validation_result.result == 'needs correction') {
      buttons = [
        {
          text: "Use recommended",
          click: function () {
            $form.removeClass('commerce_tax_services_revalidate_address');

            commerce_tax_services_clone_and_hide('input.checkout-continue');

            var selected = jQuery('#address_validation_wrapper').find('.form-type-radios.form-item-addresses input[name="addresses"]').val();
            var address = response.validation_result.suggestions[selected];

            $('fieldset.' + validation_profile + ' select.country').val(address.country);
            $('fieldset.' + validation_profile + ' input.thoroughfare').val(address.line1);
            $('fieldset.' + validation_profile + ' input.permise').val(address.line2);
            $('fieldset.' + validation_profile + ' input.locality').val(address.city);

            var $administrative_area = $('fieldset.' + validation_profile + ' select.administrative-area');
            if ($administrative_area.length > 0) {
              $administrative_area.val(address.state);
            }

            $('fieldset.' + validation_profile + ' input.postal-code').val(address.postal_code);

            $(this).dialog("close");

            $form.submit();
          }
        },
        {
          text: "Use as entered",
          click: function () {
            $form.removeClass('commerce_tax_services_revalidate_address');

            commerce_tax_services_clone_and_hide('input.checkout-continue');

            $(this).dialog('close');

            $form.submit();
          }
        },
        {
          text: "Enter again",
          click: function () {
            $(this).dialog('close');

            $('input.checkout-continue').unbind('click').click(function () {
              commerce_tax_services_clone_and_hide(this);
            });

            commerce_tax_services_attach_events();
          }
        }
      ]
    } else if (response.validation_result.result == 'invalid') {
      buttons = [
        {
          text: "Let me change the address",
          click: function () {
            $(this).dialog("close");

            $('input.checkout-continue').unbind('click').click(function () {
              commerce_tax_services_clone_and_hide(this);

              commerce_tax_services_attach_events();
            });
          }
        },
        {
          text: "Use the address anyway",
          click: function () {
            $form.removeClass('commerce_tax_services_revalidate_address');

            commerce_tax_services_clone_and_hide('input.checkout-continue');

            $form.submit();
          }
        }
      ]
    }

    var $address_validation_wrapper = $("#address_validation_wrapper");

    $address_validation_wrapper.html(response.validation_result.msg);

    $address_validation_wrapper.dialog({
      height: 500,
      width: 800,
      modal: true,
      title: Drupal.t('Confirm your shipping address'),
      resizable: false,
      draggable: false,
      buttons: buttons,
      dialogClass: 'no-close',
      closeOnEscape: false
    });

    if (!$address_validation_wrapper.dialog('isOpen')) {
      $address_validation_wrapper.dialog('open');
    }
  };
}(jQuery));
