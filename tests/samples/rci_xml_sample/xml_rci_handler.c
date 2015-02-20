#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "ccapi_xml_rci_handler.h"

//#define TEST_ERRORS

#define SET_GOOD_RESPONSE \
       "<set_setting>     \
          <serial>        \
            <baud/>       \
            <parity/>     \
            <xbreak/>     \
            <databits/>   \
          </serial>       \
          <ethernet>      \
            <ip/>         \
            <dhcp/>       \
          </ethernet>     \
        </set_setting>"

#if (defined TEST_ERRORS)
#define SET_BAD_RESPONSE  \
       "<set_setting>     \
          <serial>        \
            <baud>        \
               <error id=\"1\"> \
               <desc>linux error description while setting</desc> \
               <hint>linux error hint while setting</hint> \
               </error>   \
            </baud>       \
            <parity/>     \
            <xbreak/>     \
            <databits/>   \
          </serial>       \
          <ethernet>      \
            <ip/>         \
            <dhcp/>       \
          </ethernet>     \
        </set_setting>"
#endif

#define QUERY_SETTING_SERIAL_RESPONSE \
       "<query_setting>            \
          <serial index=\"2\">     \
            <baud>2400</baud>      \
            <parity>none</parity>  \
            <databits>5</databits> \
            <xbreak>on</xbreak>    \
            <txbytes>123</txbytes> \
          </serial>                \
        </query_setting>"

#define QUERY_SETTING_ETHERNET_RESPONSE \
       "<query_setting>            \
          <ethernet index=\"1\">   \
            <ip>0.0.0.0</ip>       \
            <subnet>0.0.0.0</subnet>     \
            <gateway>0.0.0.0</gateway>   \
            <dhcp>true</dhcp>            \
            <dns/>                       \
            <mac>00:00:00:00:00:00</mac> \
            <duplex>auto</duplex>        \
          </ethernet>                    \
        </query_setting>"

#define QUERY_SETTING_DEVICE_TIME_RESPONSE \
       "<query_setting>            \
          <device_time> \
            <curtime>2015-02-04T11:41:24+-100</curtime> \
          </device_time> \
        </query_setting>"

#define QUERY_SETTING_DEVICE_INFO_RESPONSE \
       "<query_setting>            \
          <device_info> \
            <version>0x2020000</version> \
            <product>Cloud Connector Product</product> \
            <model/> \
            <company>Digi International Inc.</company> \
            <desc>Cloud Connector Demo on Linux \
              with firmware upgrade, and remote configuration supports</desc> \
          </device_info> \
        </query_setting>"

#define QUERY_SETTING_SYSTEM_RESPONSE \
       "<query_setting>            \
          <system> \
            <description/> \
            <contact/> \
            <location/> \
          </system> \
        </query_setting>"

#define QUERY_SETTING_DEVICESECURITY_RESPONSE \
       "<query_setting>            \
          <devicesecurity> \
            <identityVerificationForm>simple</identityVerificationForm> \
          </devicesecurity> \
        </query_setting>"

#define QUERY_STATE_DEVICE_STATE_RESPONSE \
       "<query_state>            \
          <device_state> \
            <system_up_time>9</system_up_time> \
            <signed_integer>-10</signed_integer> \
            <float_value>3.56</float_value> \
          </device_state> \
        </query_state>"

#define QUERY_STATE_GPS_STATS_RESPONSE \
       "<query_state>            \
          <gps_stats> \
            <latitude>44.932017</latitude> \
            <longitude>-93.461594</longitude> \
          </gps_stats> \
        </query_state>"

#if (defined TEST_ERRORS)
#define QUERY_BAD_RESPONSE     \
       "<query_setting>        \
          <serial>             \
             <error id=\"1\">  \
             <desc>linux error description while querying</desc> \
             <hint>linux error hint while querying</hint> \
             </error>          \
          </serial>            \
        </query_setting>"
#endif

#if (defined RCI_LEGACY_COMMANDS)
#define REBOOT_GOOD_RESPONSE "<reboot/>"
#define SET_FACTORY_DEFAULTS_GOOD_RESPONSE "<set_factory_default/>"

#if (defined TEST_ERRORS)
#define REBOOT_BAD_RESPONSE \
       "<reboot>        \
             <error id=\"1\">  \
             <desc>linux error description while executing reboot</desc> \
             <hint>linux error hint while executing reboot</hint> \
             </error>          \
        </reboot>"

#define SET_FACTORY_DEFAULTS_BAD_RESPONSE \
       "<set_factory_default>        \
             <error id=\"1\">  \
             <desc>linux error description while executing set_factory_default</desc> \
             <hint>linux error hint while executing set_factory_default</hint> \
             </error>          \
        </set_factory_default>"
#endif
#endif

#define xstr(s) str(s)
#define str(s) #s
 
#if (defined TEST_ERRORS)
static unsigned int rnd_set_response = 0;
static unsigned int rnd_query_response = 0;
#if (defined RCI_LEGACY_COMMANDS)
static unsigned int rnd_do_command_response = 0;
static unsigned int rnd_reboot_response = 0;
static unsigned int rnd_set_factory_default_response = 0;
#endif
#endif

static int get_request_buffer(char * * xml_request_buffer)
{
    int error_id = 0;
    struct stat request_info;
    FILE * xml_request_fp = NULL;

    stat(XML_REQUEST_FILE_NAME, &request_info);

    *xml_request_buffer = malloc(request_info.st_size + 1);

    if (*xml_request_buffer == NULL)
    {
        error_id = -1;
        goto done;
    }

    xml_request_fp = fopen(XML_REQUEST_FILE_NAME, "r");
    if (xml_request_fp == NULL)
    {
        printf("%s: Unable to open %s file\n", __FUNCTION__,  XML_REQUEST_FILE_NAME);
        error_id = -1;
        goto done;
    }          

    fread(*xml_request_buffer, 1, request_info.st_size, xml_request_fp);
    (*xml_request_buffer)[request_info.st_size] = '\0';
    if (ferror(xml_request_fp) != 0)
    {
        printf("%s: Failed to read %s file\n", __FUNCTION__, XML_REQUEST_FILE_NAME);

        error_id = -2;
        goto done;
    }

    /* printf("request:\n%s\n", *xml_request_buffer); */

    fclose(xml_request_fp);

done:
    return error_id;
}


void xml_rci_handler(void)
{
    char * xml_request_buffer = NULL;
    FILE * xml_response_fp = NULL;
    char * group_ptr = NULL;

    printf("    Called '%s'\n", __FUNCTION__);

    if (get_request_buffer(&xml_request_buffer) != 0)
    {
        goto done;
    }

    xml_response_fp = fopen(XML_RESPONSE_FILE_NAME, "w+");
    if (xml_response_fp == NULL)
    {
        printf("%s: Unable to open %s file\n", __FUNCTION__,  XML_RESPONSE_FILE_NAME);
        goto done;
    }

    /* process the XML_REQUEST_FILE_NAME file and provide a response at XML_RESPONSE_FILE_NAME */
    if (strncmp(xml_request_buffer, "<query_setting", sizeof("<query_setting") - 1) == 0)
    {
        group_ptr = strstr(xml_request_buffer, "   <");
        if (group_ptr != NULL)
        {
            group_ptr += sizeof("   ") - 1;
            if (strncmp(group_ptr, "<serial", sizeof("<serial") - 1) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_SERIAL_RESPONSE);
            }
            else if (strncmp(group_ptr, "<ethernet", sizeof("<ethernet") -1 ) == 0)
            {
                /* Just a test: every two query request for the group 'ethernet' return an error */
#if (defined TEST_ERRORS)
                if (rnd_query_response++ % 2)
                    fprintf(xml_response_fp, "%s", QUERY_BAD_RESPONSE);
                else
#endif
                    fprintf(xml_response_fp, "%s", QUERY_SETTING_ETHERNET_RESPONSE);
            }
            else if (strncmp(group_ptr, "<device_time", sizeof("<device_time") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_DEVICE_TIME_RESPONSE);
            }
            else if (strncmp(group_ptr, "<device_info", sizeof("<device_info") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_DEVICE_INFO_RESPONSE);
            }
            else if (strncmp(group_ptr, "<system", sizeof("<system") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_SYSTEM_RESPONSE);
            }
            else if (strncmp(group_ptr, "<devicesecurity", sizeof("<devicesecurity") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_DEVICESECURITY_RESPONSE);
            }
        }
    }
    else if (strncmp(xml_request_buffer, "<query_state", sizeof("<query_state") - 1) == 0)
    {
        group_ptr = strstr(xml_request_buffer, "   <");
        if (group_ptr != NULL)
        {
            group_ptr += sizeof("   ") - 1;
            if (strncmp(group_ptr, "<device_state", sizeof("<device_state") - 1) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_STATE_DEVICE_STATE_RESPONSE);
            }
            else if (strncmp(group_ptr, "<gps_stats", sizeof("<gps_stats") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_STATE_GPS_STATS_RESPONSE);
            }
        }
    }
    else if (strncmp(xml_request_buffer, "<set_setting", sizeof("<set_setting") - 1) == 0)
    {
        /* Don't mind the group set in the request... just provide a response (for 'ethernet' group for example)
           with or without 'error' tag randomly */
#if (defined TEST_ERRORS)
        if (rnd_set_response++ % 2)
            fprintf(xml_response_fp, "%s", SET_BAD_RESPONSE);
        else
#endif
            fprintf(xml_response_fp, "%s", SET_GOOD_RESPONSE);
    }
    else if (strncmp(xml_request_buffer, "<set_state", sizeof("<set_state") - 1) == 0)
    {
        /* Don't mind the group set in the request... just provide a response (for 'ethernet' group for example) */
        fprintf(xml_response_fp, "%s", SET_GOOD_RESPONSE);
    }
#if (defined RCI_LEGACY_COMMANDS)
    /* do_command without target */
    #define DO_COMMAND_NO_TARGET "<do_command>"
    else if (strncmp(xml_request_buffer, DO_COMMAND_NO_TARGET, sizeof(DO_COMMAND_NO_TARGET) - 1) == 0)
    {
#if (defined TEST_ERRORS)
        if (rnd_do_command_response % 3 == 2)
        {

            fprintf(xml_response_fp, "<do_command> \
                                         <error id=\"1\">  \
                                            <desc>linux error description while executing do_command</desc> \
                                            <hint>linux error hint while executing do_command without target</hint> \
                                         </error>          \
                                      </do_command>");
        }
        else
#endif
        {
            if (rnd_do_command_response % 3 == 1)
            {
                /* Empty response */
                fprintf(xml_response_fp, "<do_command/>");
            }
            else
            {
                fprintf(xml_response_fp, "<do_command>do command without target good response</do_command>");
            }
        }
        rnd_do_command_response++;
    }
    /* do_command with target */
    #define DO_COMMAND_TARGET "<do_command target=\""
    else if (strncmp(xml_request_buffer, DO_COMMAND_TARGET, sizeof(DO_COMMAND_TARGET) - 1) == 0)
    {
        char const * const target_ptr = &xml_request_buffer[sizeof(DO_COMMAND_TARGET) - 1];
        int scanf_ret;
        char target[RCI_COMMANDS_ATTRIBUTE_MAX_LEN + 1];

        #define RCI_COMMANDS_ATTRIBUTE_MAX_LEN_STR  xstr(RCI_COMMANDS_ATTRIBUTE_MAX_LEN)

        #define TARGET_FORMAT "%" RCI_COMMANDS_ATTRIBUTE_MAX_LEN_STR "[^\"]>"
        scanf_ret = sscanf(target_ptr, TARGET_FORMAT, target);
        if (scanf_ret == 0)
        {
            goto done;
        }

        /* printf("target='%s'\n", target); */

#if (defined TEST_ERRORS)
        if (rnd_do_command_response % 3 == 2)
        {
            fprintf(xml_response_fp, "<do_command target=\"%s\"> \
                                         <error id=\"1\">  \
                                            <desc>linux error description while executing do_command</desc> \
                                            <hint>linux error hint while executing do_command with target</hint> \
                                         </error>          \
                                      </do_command>", target);
        }
        else
#endif
        {
            if (rnd_do_command_response % 3 == 1)
            {
                /* Empty response */
                fprintf(xml_response_fp, "<do_command target=\"%s\"/>", target);
            }
            else
            {
                fprintf(xml_response_fp, "<do_command target=\"%s\">do command with target good response</do_command>", target);
            }
        }
        rnd_do_command_response++;
    }
    else if (strncmp(xml_request_buffer, "<set_factory_default", sizeof("<set_factory_default") - 1) == 0)
    {
        /* Just provide a response with or without 'error' tag randomly */
#if (defined TEST_ERRORS)
        if (rnd_set_factory_default_response++ % 2)
            fprintf(xml_response_fp, "%s", SET_FACTORY_DEFAULTS_BAD_RESPONSE);
        else
#endif
            fprintf(xml_response_fp, "%s", SET_FACTORY_DEFAULTS_GOOD_RESPONSE);
    }
    else if (strncmp(xml_request_buffer, "<reboot", sizeof("<reboot") - 1) == 0)
    {
        /* Just provide a response with or without 'error' tag randomly */
#if (defined TEST_ERRORS)
        if (rnd_reboot_response++ % 2)
            fprintf(xml_response_fp, "%s", REBOOT_BAD_RESPONSE);
        else
#endif
            fprintf(xml_response_fp, "%s", REBOOT_GOOD_RESPONSE);
    }
#endif
    else
    {
        printf("Unsupported command!!!!!\n");
        assert(0);
    }

done:
    if (xml_response_fp != NULL)
        fclose(xml_response_fp);

    if (xml_request_buffer != NULL)
        free(xml_request_buffer);

    return;
}

