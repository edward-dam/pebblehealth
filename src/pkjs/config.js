//Author: Ed Dam

module.exports = [
  { "type": "heading", "defaultValue": "Health v1.0" },
  { "type": "text", "defaultValue": "by Edward Dam" },
  { "type": "section", "items": [
    { "type": "heading", "defaultValue": "Time" },
    { "type": "text", "defaultValue": "Please Choose 24 Hours or 12 Hours" },
    { "type": "radiogroup", "messageKey": "HOUR", "options": [
      { "label": "24 Hours", "value": "false" },
      { "label": "12 Hours", "value": "true" } ],
    "defaultValue": "false" } ]
  },
  { "type": "section", "items": [
    { "type": "heading", "defaultValue": "Date" },
    { "type": "text", "defaultValue": "Please Choose Date Order" },
    { "type": "radiogroup", "messageKey": "DATE", "options": [
      { "label": "Day - Month", "value": "false" },
      { "label": "Month - Day", "value": "true" } ],
    "defaultValue": "false" } ]
  },
  { "type": "text", "defaultValue": "Thank you for using my watchface." },
  { "type": "submit", "defaultValue": "Save Settings" }
];
