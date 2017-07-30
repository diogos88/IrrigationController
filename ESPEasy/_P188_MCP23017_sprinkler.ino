//#######################################################################################################
//################################# Plugin 188: Sprinkler Controller ####################################
//#######################################################################################################

#define PLUGIN_188
#define PLUGIN_ID_188         188
#define PLUGIN_NAME_188       "Sprinkler Controller - MCP23017"
#define PLUGIN_VALUENAME1_188 "SprinklerController"

#define NumRelays 8

boolean Plugin_188(byte function, struct EventStruct *event, String& string)
{
   boolean success = false;

   switch (function)
   {
   case PLUGIN_DEVICE_ADD:
   {
      Device[++deviceCount].Number = PLUGIN_ID_188;
      Device[deviceCount].Type = DEVICE_TYPE_I2C;
      Device[deviceCount].VType = SENSOR_TYPE_OCTA;
      Device[deviceCount].Ports = 0;
      Device[deviceCount].PullUpOption = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption = false;
      Device[deviceCount].ValueCount = NumRelays;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].TimerOptional = true;
      Device[deviceCount].GlobalSyncOption = true;
      Device[deviceCount].DecimalsOnly = false;
      break;
   }

   case PLUGIN_GET_DEVICENAME:
   {
      string = F(PLUGIN_NAME_188);
      break;
   }

   case PLUGIN_GET_DEVICEVALUENAMES:
   {
      for (int i = 0; i < NumRelays; i++)
      {
         String name = PLUGIN_VALUENAME1_188;
         name += (i + 1);

         name.toCharArray(ExtraTaskSettings.TaskDeviceValueNames[i], 41);
      }
      break;
   }

   case PLUGIN_WEBFORM_LOAD:
   {
      String header = F("MCP pins");
      addFormSubHeader(string, header);

      addFormNumericBox(string, F("Relay 1"), F("plugin_188_relay1"), ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
      addFormNumericBox(string, F("Relay 2"), F("plugin_188_relay2"), ExtraTaskSettings.TaskDevicePluginConfigLong[1]);
      addFormNumericBox(string, F("Relay 3"), F("plugin_188_relay3"), ExtraTaskSettings.TaskDevicePluginConfigLong[2]);
      addFormNumericBox(string, F("Relay 4"), F("plugin_188_relay4"), ExtraTaskSettings.TaskDevicePluginConfigLong[3]);
      addFormNumericBox(string, F("Relay 5"), F("plugin_188_relay5"), ExtraTaskSettings.TaskDevicePluginConfigLong[4]);
      addFormNumericBox(string, F("Relay 6"), F("plugin_188_relay6"), ExtraTaskSettings.TaskDevicePluginConfigLong[5]);
      addFormNumericBox(string, F("Relay 7"), F("plugin_188_relay7"), ExtraTaskSettings.TaskDevicePluginConfigLong[6]);
      addFormNumericBox(string, F("Relay 8"), F("plugin_188_relay8"), ExtraTaskSettings.TaskDevicePluginConfigLong[7]);

      success = true;
      break;
   }

   case PLUGIN_WEBFORM_SAVE:
   {
      ExtraTaskSettings.TaskDevicePluginConfigLong[0] = getFormItemInt(F("plugin_188_relay1"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[1] = getFormItemInt(F("plugin_188_relay2"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[2] = getFormItemInt(F("plugin_188_relay3"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[3] = getFormItemInt(F("plugin_188_relay4"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[4] = getFormItemInt(F("plugin_188_relay5"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[5] = getFormItemInt(F("plugin_188_relay6"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[6] = getFormItemInt(F("plugin_188_relay7"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[7] = getFormItemInt(F("plugin_188_relay8"));

      success = true;
      break;
   }

   case PLUGIN_INIT:
   {
      LoadTaskSettings(event->TaskIndex);

      for (int i = 0; i < NumRelays; i++)
      {
         Plugin_188_ConfigOutputMCP(i + 1, 1);

         UserVar[event->BaseVarIndex + i] = 0;
         Plugin_188_WriteToMCP(ExtraTaskSettings.TaskDevicePluginConfigLong[i], UserVar[event->BaseVarIndex + i]);
         setPinState(PLUGIN_ID_188, ExtraTaskSettings.TaskDevicePluginConfigLong[i], PIN_MODE_OUTPUT, UserVar[event->BaseVarIndex + i]);
      }

      success = true;
      break;
   }

   case PLUGIN_TEN_PER_SECOND:
   {
      for (int i = 0; i < NumRelays; i++)
      {
         int state = Plugin_188_ReadFromMCP(ExtraTaskSettings.TaskDevicePluginConfigLong[i]);
         if ((state != -1) && (state != UserVar[event->BaseVarIndex + i]))
         {
            UserVar[event->BaseVarIndex + i] = state;

            String log = F("");
            String msg = getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_188, ExtraTaskSettings.TaskDevicePluginConfigLong[i], log, 0);
            Plugin_188_SendToController(msg);
         }
      }

      success = true;
      break;
   }

   case PLUGIN_READ:
   {
      String log = F("");
      for (int i = 0; i < NumRelays; i++)
      {
         if (UserVar[event->BaseVarIndex + i])
         {
            String msg = getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_188, ExtraTaskSettings.TaskDevicePluginConfigLong[i], log, 0);
            Plugin_188_SendToController(msg);
         }
      }

      success = true;
      break;
   }

   case PLUGIN_WRITE:
   {
      String log = "";
      String command = parseString(string, 1);

      if (command == F("status"))
      {
         if (parseString(string, 2) == F("mcp"))
         {
            String status = "";
            if (hasPinState(PLUGIN_ID_188, event->Par2))  // has been set as output
               status = getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_188, event->Par2, dummyString, 0);
            else
            {
               int state = Plugin_188_ReadFromMCP(event->Par2); // report as input
               if (state != -1)
                  status = getPinStateJSON(NO_SEARCH_PIN_STATE, PLUGIN_ID_188, event->Par2, dummyString, state);
            }
            Plugin_188_SendToController(status);
            success = true;
         }
      }
      else
      {
         //Serial.println("Command: mcpasyncpulse");
         //Serial.print("TaskIndex: ");
         //Serial.println(event->TaskIndex);
         //Serial.print("Par1: ");
         //Serial.println(event->Par1);
         //Serial.print("Par2: ");
         //Serial.println(event->Par2);
         //Serial.print("Par3: ");
         //Serial.println(event->Par3);

         // respond only if the pin is in use with the current configuration
         for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
         {
            //Serial.print("LOOP plugin: ");
            //Serial.println(pinStates[x].plugin);
            //Serial.print("LOOP index: ");
            //Serial.println(pinStates[x].index == event->Par1);

            if ((pinStates[x].plugin == PLUGIN_ID_188) && (pinStates[x].index == event->Par1))
            {
               if (command == F("mcpasyncpulse"))
               {
                  Serial.println("mcpasyncpulse found");

                  Plugin_188_Write(event->TaskIndex, event->Par1, event->Par2, _max(0, event->Par3));
                  success = true;
                  break;
               }
               if (command == F("mcpasyncpulsedefault"))
               {
                  if (event->Par3 > 0 && event->Par3 <= NumRelays)
                  {
                     long timer = ExtraTaskSettings.TaskDevicePluginConfigLong[event->Par3 - 1];
                     Plugin_188_Write(event->TaskIndex, event->Par1, event->Par2, _max(0, timer));
                  }
                  else
                  {
                     Plugin_188_SendToController(F("?"));
                  }

                  success = true;
                  break;
               }
               if (command == F("mcpgpio"))
               {
                  Plugin_188_Write(event->Source, event->Par1, event->Par2, 0);
                  success = true;
                  break;
               }

               if (command == F("mcppulse"))
               {
                  Plugin_188_Write(event->TaskIndex, event->Par1, event->Par2, 0);
                  delay(event->Par3 * 1000);
                  Plugin_188_Write(event->TaskIndex, event->Par1, !event->Par2, 0);
                  success = true;
                  break;
               }
            }
         }
      }

      break;
   }

   case PLUGIN_TIMER_IN:
   {
      Plugin_188_Write(event->Par3, event->Par1, event->Par2, 0);
      break;
   }
   }
   return success;
}


int Plugin_188_ReadFromMCP(byte pin)
{
   int8_t state = -1;
   byte unit = (pin - 1) / 16;
   byte port = pin - (unit * 16);
   uint8_t address = 0x20 + unit;
   byte IOBankValueReg = 0x12;
   if (port > 8)
   {
      port = port - 8;
      IOBankValueReg++;
   }
   // get the current pin status
   Wire.beginTransmission(address);
   Wire.write(IOBankValueReg); // IO data register
   Wire.endTransmission();
   Wire.requestFrom(address, (uint8_t)0x1);
   if (Wire.available())
   {
      state = ((Wire.read() & _BV(port - 1)) >> (port - 1));
   }
   return state;
}

boolean Plugin_188_WriteToMCP(byte pin, byte state)
{
   boolean success = false;
   byte portvalue = 0;
   byte unit = (pin - 1) / 16;
   byte port = pin - (unit * 16);
   uint8_t address = 0x20 + unit;
   byte IOBankConfigReg = 0;
   byte IOBankValueReg = 0x12;
   if (port > 8)
   {
      port = port - 8;
      IOBankConfigReg++;
      IOBankValueReg++;
   }
   // turn this port into output, first read current config
   Wire.beginTransmission(address);
   Wire.write(IOBankConfigReg); // IO config register
   Wire.endTransmission();
   Wire.requestFrom(address, (uint8_t)0x1);
   if (Wire.available())
   {
      portvalue = Wire.read();
      portvalue &= ~(1 << (port - 1)); // change pin from (default) input to output

      // write new IO config
      Wire.beginTransmission(address);
      Wire.write(IOBankConfigReg); // IO config register
      Wire.write(portvalue);
      Wire.endTransmission();
   }
   // get the current pin status
   Wire.beginTransmission(address);
   Wire.write(IOBankValueReg); // IO data register
   Wire.endTransmission();
   Wire.requestFrom(address, (uint8_t)0x1);
   if (Wire.available())
   {
      portvalue = Wire.read();
      if (state == 1)
         portvalue |= (1 << (port - 1));
      else
         portvalue &= ~(1 << (port - 1));

      // write back new data
      Wire.beginTransmission(address);
      Wire.write(IOBankValueReg);
      Wire.write(portvalue);
      Wire.endTransmission();
      success = true;
   }
}

boolean Plugin_188_ConfigOutputMCP(byte pin, byte state)
{
   boolean success = false;
   byte portvalue = 0;
   byte unit = (pin - 1) / 16;
   byte port = pin - (unit * 16);
   uint8_t address = 0x20 + unit;
   byte IOBankConfigReg = 0xC;
   if (port > 8)
   {
      port = port - 8;
      IOBankConfigReg++;
   }
   // turn this port pullup on
   Wire.beginTransmission(address);
   Wire.write(IOBankConfigReg);
   Wire.endTransmission();
   Wire.requestFrom(address, (uint8_t)0x1);
   if (Wire.available())
   {
      portvalue = Wire.read();
      if (state == 1)
         portvalue |= (1 << (port - 1));
      else
         portvalue &= ~(1 << (port - 1));

      // write new IO config
      Wire.beginTransmission(address);
      Wire.write(IOBankConfigReg); // IO config register
      Wire.write(portvalue);
      Wire.endTransmission();
   }
}

void Plugin_188_Write(byte taskIndex, byte pin, byte state, uint16_t timer)
{
   Plugin_188_WriteToMCP(pin, state);
   setPinState(PLUGIN_ID_188, pin, PIN_MODE_OUTPUT, state);


   if (timer > 0)
   {
      setSystemTimer(timer * 1000, PLUGIN_ID_188, pin, !state, taskIndex);
   }
   else
   {
      // Check to deactivate the timer
      for (byte x = 0; x < SYSTEM_TIMER_MAX; x++)
      {
         if (systemTimers[x].timer != 0)
         {
            if ((systemTimers[x].plugin == PLUGIN_ID_188) && (systemTimers[x].Par1 == pin))
            {
               systemTimers[x].plugin = 0;
               systemTimers[x].timer = 0;
               break;
            }
         }
      }
   }

   // Force sync
   struct EventStruct TempEvent;
   TempEvent.TaskIndex = taskIndex;
   TempEvent.BaseVarIndex = taskIndex * VARS_PER_TASK;
   byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
   TempEvent.sensorType = Device[DeviceIndex].VType;
   TempEvent.OriginTaskIndex = taskIndex;

   LoadTaskSettings(taskIndex);

   Plugin_188(PLUGIN_TEN_PER_SECOND, &TempEvent, dummyString);
   SensorSendTask(taskIndex);
}

void Plugin_188_SendToController(String message)
{
   SendStatus(VALUE_SOURCE_HTTP, message);
   SendStatus(VALUE_SOURCE_MQTT, message);
}