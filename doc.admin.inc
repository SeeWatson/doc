<?php

/**
 * @file
 * Administrative page callbacks for the googleanalytics module.
 */

/**
 * Implements hook_admin_settings() for module settings configuration.
 */
function doc_admin_settings_form($form,&$form_state) {
	$form['entities'] = array(
		'#type' => 'fieldset',
		'#title' => t('Documentation Module Settings'),
	);

	$form['entities']['doc_what'] = array(
	'#type' => 'checkboxes',
	'#title' => t('What to Document'),
		'#options' => array(
			'dr' => t('Drupal Implementation Overview'),
			'db' => t('Database'),
			'ct' => t('Content Types'),
			'md' => t('Modules'),
			'tx' => t('Taxonomies'),
			'th' => t('Themes'),
			'var' => t('System Variables'),
			'bl' => t('Blocks'),
			'us' => t('Users'),
			'ur' => t('User Roles'),
		),
	);
	
	return system_settings_form($form);
}
function doc_documentation() {
	return "Testing";
}
function doc_admin_settings_validate($form, &$form_state) {
	// Validation logic.
}
function doc_admin_settings_submit($form, &$form_state) {
	// Submission logic.
}