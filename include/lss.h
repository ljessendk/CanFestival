#if !defined(_LSS_H_)
#define _LSS_H_


void lss_SwitchModeGlobal(CO_Data *d, UNS32 mode);
void lss_SwitchModeSelective(CO_Data *d, UNS32 *LSSaddr);
void lss_ConfigureNode_ID(CO_Data *d, UNS32 node_id);

void lss_ConfigureBitTimingParameters(CO_Data *d, 
                                      UNS32 table_selector, 
                                      UNS32 table_index);

void lss_ActivateBitTimingParameters_master(CO_Data *d, UNS16 switch_delay);
void lss_ActivateBitTimingParameters_slave(UNS8 data1, UNS8 data2);

void lss_StoreConfiguredParameters(CO_Data *d);
UNS32 lss_InquireLSSAddress(CO_Data *d, UNS32 *LSSaddr);

void lss_IdentifyRemoteSlaves(CO_Data *d, 
                              UNS32 vendor_id,
                              UNS32 product_code,
                              UNS32 rev_low,
                              UNS32 rev_high,
                              UNS32 serial_low,
                              UNS32 serial_high);

UNS32 lss_validate_address(CO_Data *d);

void lss_IdentifySlave(CO_Data *d);


#endif

