//#######################################################################################################
//########################### Controller Plugin 005: OpenHAB MQTT #######################################
//#######################################################################################################

#define CPLUGIN_005
#define CPLUGIN_ID_005         5
#define CPLUGIN_NAME_005       "OpenHAB MQTT"

boolean CPlugin_005(byte function, struct EventStruct *event, String& string)
{
   boolean success = false;

   switch (function)
   {
   case CPLUGIN_PROTOCOL_ADD:
   {
      Protocol[++protocolCount].Number = CPLUGIN_ID_005;
      Protocol[protocolCount].usesMQTT = true;
      Protocol[protocolCount].usesTemplate = true;
      Protocol[protocolCount].usesAccount = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].defaultPort = 1883;
      Protocol[protocolCount].usesID = false;
      break;
   }

   case CPLUGIN_GET_DEVICENAME:
   {
      string = F(CPLUGIN_NAME_005);
      break;
   }

   case CPLUGIN_PROTOCOL_TEMPLATE:
   {
      event->String1 = F("/%sysname%/#");
      event->String2 = F("/%sysname%/%tskname%");
      break;
   }

   case CPLUGIN_PROTOCOL_RECV:
   {
      // Split topic into array
      String tmpTopic = event->String1.substring(1);
      String topicSplit[10];
      int SlashIndex = tmpTopic.indexOf('/');
      byte count = 0;
      while (SlashIndex > 0 && count < 10 - 1)
      {
         topicSplit[count] = tmpTopic.substring(0, SlashIndex);
         tmpTopic = tmpTopic.substring(SlashIndex + 1);
         SlashIndex = tmpTopic.indexOf('/');
         count++;
      }
      topicSplit[count] = tmpTopic;

      String cmd = "";
      struct EventStruct TempEvent;

      if (topicSplit[count] == F("cmd"))
      {
         cmd = event->String2;
         parseCommandString(&TempEvent, cmd);
         TempEvent.Source = VALUE_SOURCE_MQTT;
      }
      else
      {
         cmd = topicSplit[count - 1];
         TempEvent.Par1 = topicSplit[count].toInt();
         TempEvent.Par2 = event->String2.toFloat();
         TempEvent.Par3 = 0;
      }
      // in case of event, store to buffer and return...
      String command = parseString(cmd, 1);
      if (command == F("event"))
         eventBuffer = cmd.substring(6);
      else if
         (PluginCall(PLUGIN_WRITE, &TempEvent, cmd));
      else
         remoteConfig(&TempEvent, cmd);

      break;
   }

   case CPLUGIN_PROTOCOL_SEND:
   {
      ControllerSettingsStruct ControllerSettings;
      LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));

      statusLED(true);

      if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
         PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

      String pubname = ControllerSettings.Publish;
      pubname.replace(F("%sysname%"), Settings.Name);
      pubname.replace(F("%tskname%"), ExtraTaskSettings.TaskDeviceName);
      pubname.replace(F("%id%"), String(event->idx));

      //Serial.print("pubname: ");
      //Serial.println(pubname);

      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[event->TaskIndex]);
      byte valueCount = getValueCountFromSensorType(event->sensorType);

      //Serial.print("Device Index: ");
      //Serial.println(DeviceIndex);
      //Serial.print("ValueCount: ");
      //Serial.println(valueCount);

      String payload = "{";

      for (byte x = 0; x < valueCount; x++)
      {
         //Serial.print("LOOP value idx: ");
         //Serial.println(x);

         payload += "\"";
         payload += ExtraTaskSettings.TaskDeviceValueNames[x];
         payload += "\":";

         if (event->sensorType == SENSOR_TYPE_LONG)
            payload += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
         else
            payload += toString(UserVar[event->BaseVarIndex + x], ExtraTaskSettings.TaskDeviceValueDecimals[x]);

         if (x < (valueCount - 1))
            payload += ",";

         //Serial.print("LOOP payload: ");
         //Serial.println(payload);
      }

      payload += "}";

      //Serial.print("FULL PAYLOAD: ");
      //Serial.println(payload);

      MQTTclient.publish(pubname.c_str(), payload.c_str(), Settings.MQTTRetainFlag);

      //Serial.println("MSG SENT");

      String log = F("MQTT : ");
      log += pubname;
      log += " ";
      log += payload;
      addLog(LOG_LEVEL_DEBUG, log);
      break;
   }
   return success;
   }
}
