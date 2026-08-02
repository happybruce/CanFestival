#include <linux/module.h>
#include "data.h"
#include "can_driver.h"
#include "dcf.h"
#include "nmtSlave.h"
#include "timers_driver.h"

// CanFestival symbols available to other kernel modules

// dcf.h
EXPORT_SYMBOL (send_consise_dcf);

// emcy.h
EXPORT_SYMBOL (dummy_post_emcy);
EXPORT_SYMBOL (EMCY_setError);
EXPORT_SYMBOL (EMCY_errorRecovered);
EXPORT_SYMBOL (emergencyInit);
EXPORT_SYMBOL (emergencyStop);
EXPORT_SYMBOL (proceedEMCY);

// lifegrd.h
EXPORT_SYMBOL (dummy_heartbeatError);
EXPORT_SYMBOL (dummy_post_SlaveBootup);
EXPORT_SYMBOL (getNodeState);
EXPORT_SYMBOL (heartbeatInit);
EXPORT_SYMBOL (heartbeatStop);
EXPORT_SYMBOL (proceedNODE_GUARD);

// lss.h
#ifdef CO_ENABLE_LSS
EXPORT_SYMBOL (startLSS);
EXPORT_SYMBOL (stopLSS);
EXPORT_SYMBOL (sendLSS);
EXPORT_SYMBOL (sendLSSMessage);
EXPORT_SYMBOL (proceedLSS_Master);
EXPORT_SYMBOL (proceedLSS_Slave);
EXPORT_SYMBOL (configNetworkNode);
EXPORT_SYMBOL (configNetworkNodeCallBack);
EXPORT_SYMBOL (getConfigResultNetworkNode);
#endif

// nmtMaster.h
EXPORT_SYMBOL (masterSendNMTstateChange);
EXPORT_SYMBOL (masterSendNMTnodeguard);
EXPORT_SYMBOL (masterRequestNodeState);

// nmtSlave.h
EXPORT_SYMBOL (proceedNMTstateChange);
EXPORT_SYMBOL (slaveSendBootUp);

// objacces.h
EXPORT_SYMBOL (dummy_storeODSubIndex);
#ifdef DEBUG_WAR_CONSOLE_ON
EXPORT_SYMBOL (accessDictionaryError);
#endif
EXPORT_SYMBOL (_getODentry);
EXPORT_SYMBOL (_setODentry);
//EXPORT_SYMBOL (writeLocalDict);
//EXPORT_SYMBOL (scanIndexOD);
EXPORT_SYMBOL (RegisterSetODentryCallBack);

// pdo.h
EXPORT_SYMBOL (buildPDO);
EXPORT_SYMBOL (sendPDOrequest);
EXPORT_SYMBOL (proceedPDO);
EXPORT_SYMBOL (sendPDOevent);
EXPORT_SYMBOL (sendOnePDOevent);
EXPORT_SYMBOL (_sendPDOevent);
EXPORT_SYMBOL (PDOInit);
EXPORT_SYMBOL (PDOStop);
EXPORT_SYMBOL (PDOEventTimerAlarm);
EXPORT_SYMBOL (PDOInhibitTimerAlarm);
EXPORT_SYMBOL (CopyBits);

// sdo.h
EXPORT_SYMBOL (SDOTimeoutAlarm);
EXPORT_SYMBOL (resetSDO);
EXPORT_SYMBOL (SDOLineToObjDict);
EXPORT_SYMBOL (objDictToSDOLine);
EXPORT_SYMBOL (lineToSDO);
EXPORT_SYMBOL (SDOtoLine);
EXPORT_SYMBOL (failedSDO);
EXPORT_SYMBOL (resetSDOLine);
EXPORT_SYMBOL (initSDOLine);
EXPORT_SYMBOL (getSDOFreeLine);
EXPORT_SYMBOL (getSDOLineOnUse);
EXPORT_SYMBOL (closeSDOTransfer);
EXPORT_SYMBOL (getSDOLineRestBytes);
EXPORT_SYMBOL (setSDOLineRestBytes);
EXPORT_SYMBOL (sendSDO);
EXPORT_SYMBOL (sendSDOAbort);
EXPORT_SYMBOL (proceedSDO);
EXPORT_SYMBOL (writeNetworkDict);
EXPORT_SYMBOL (writeNetworkDictCallBack);
EXPORT_SYMBOL (readNetworkDict);
EXPORT_SYMBOL (readNetworkDictCallback);
EXPORT_SYMBOL (getReadResultNetworkDict);
EXPORT_SYMBOL (getWriteResultNetworkDict);

// states.h
EXPORT_SYMBOL (dummy_initialisation);
EXPORT_SYMBOL (default_preOperational);
EXPORT_SYMBOL (dummy_operational);
EXPORT_SYMBOL (dummy_stopped);
EXPORT_SYMBOL (canDispatch);
EXPORT_SYMBOL (getState);
EXPORT_SYMBOL (setState);
EXPORT_SYMBOL (getNodeId);
EXPORT_SYMBOL (setNodeId);

// sync.h
EXPORT_SYMBOL (startSYNC);
EXPORT_SYMBOL (stopSYNC);
EXPORT_SYMBOL (dummy_post_sync);
EXPORT_SYMBOL (dummy_post_TPDO);
EXPORT_SYMBOL (sendSYNC);
EXPORT_SYMBOL (sendSYNCMessage);
EXPORT_SYMBOL (proceedSYNC);

// timer.h
EXPORT_SYMBOL (SetAlarm);
EXPORT_SYMBOL (DelAlarm);
EXPORT_SYMBOL (TimeDispatch);
EXPORT_SYMBOL (setTimer);
EXPORT_SYMBOL (getElapsedTime);

// timers_driver.h
EXPORT_SYMBOL (EnterMutex);
EXPORT_SYMBOL (LeaveMutex);
EXPORT_SYMBOL (TimerInit);
EXPORT_SYMBOL (TimerCleanup);
EXPORT_SYMBOL (StartTimerLoop);
EXPORT_SYMBOL (StopTimerLoop);
EXPORT_SYMBOL (CreateReceiveTask);
EXPORT_SYMBOL (WaitReceiveTaskEnd);
