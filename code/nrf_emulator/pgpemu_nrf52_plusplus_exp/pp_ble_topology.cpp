#include "pp_ble_topology.h"

void ppSetupPokemonGoPlusTopology(const BleTopologyRefs& refs, const BleSetupCallbacks& callbacks)
{
  refs.serviceA->begin();
  ppSetupWritable(*refs.serviceAChar1, CHR_PROPS_WRITE_WO_RESP, refs.shortValueLen, callbacks);
  ppSetupWritable(*refs.serviceAChar2, CHR_PROPS_WRITE_WO_RESP, refs.shortValueLen, callbacks);
  refs.serviceAChar3->setProperties(CHR_PROPS_READ);
  ppSetupReadableWithTrace(*refs.serviceAChar3, refs.serviceARead3, refs.serviceARead3Len, callbacks);
  refs.serviceAChar4->setProperties(CHR_PROPS_READ);
  ppSetupReadableWithTrace(*refs.serviceAChar4, refs.serviceARead4, refs.serviceARead4Len, callbacks);
  ppSetupNotifyReadable(*refs.serviceAChar5,
                        CHR_PROPS_READ | CHR_PROPS_NOTIFY,
                        refs.serviceAStatus,
                        refs.serviceAStatusLen,
                        callbacks);

  refs.batteryService->begin();
  refs.batteryLevelChar->setProperties(CHR_PROPS_READ);
  ppSetupReadableWithTrace(*refs.batteryLevelChar,
                           refs.batteryLevelDefault,
                           refs.batteryLevelDefaultLen,
                           callbacks);

  refs.serviceB->begin();
  ppSetupNotifyVariable(*refs.serviceBChar1,
                        CHR_PROPS_READ | CHR_PROPS_NOTIFY,
                        refs.serviceBStatus1,
                        refs.serviceBStatus1Len,
                        refs.serviceBStatus1Len,
                        callbacks);
  ppSetupNotifyReadable(*refs.serviceBChar2,
                        CHR_PROPS_READ | CHR_PROPS_NOTIFY,
                        refs.serviceBStatus2,
                        refs.serviceBStatus2Len,
                        callbacks);
  refs.serviceBChar3->setProperties(CHR_PROPS_READ);
  ppSetupReadable(*refs.serviceBChar3, refs.serviceBRead3, refs.serviceBRead3Len);

  refs.serviceC->begin();
  ppSetupNotifyVariable(*refs.serviceCChar1,
                        CHR_PROPS_WRITE | CHR_PROPS_NOTIFY,
                        refs.serviceCState,
                        refs.serviceCStateLen,
                        refs.mediumValueLen,
                        callbacks);
  refs.serviceCChar1->setWriteCallback(callbacks.serviceWriteCallback);
  ppSetupWritable(*refs.serviceCChar2, CHR_PROPS_WRITE_WO_RESP, refs.mediumValueLen, callbacks);

  refs.serviceD->begin();
  ppSetupWritable(*refs.serviceDChar1, CHR_PROPS_WRITE, refs.mediumValueLen, callbacks);
  ppSetupNotifyVariable(*refs.serviceDChar2,
                        CHR_PROPS_READ | CHR_PROPS_NOTIFY,
                        refs.serviceDStatus,
                        refs.serviceDStatusLen,
                        refs.serviceDReadBlobMaxLen,
                        callbacks);
  refs.serviceDChar3->setProperties(CHR_PROPS_READ);
  ppSetupReadableVariableWithTrace(*refs.serviceDChar3,
                                   refs.serviceDReadBlob,
                                   refs.serviceDReadBlobInitialLen,
                                   refs.serviceDReadBlobMaxLen,
                                   callbacks);

  refs.serviceE->begin();
  ppSetupWritable(*refs.ledWriteChar, CHR_PROPS_WRITE, refs.mediumValueLen, callbacks);
  ppSetupNotifyReadable(*refs.buttonNotifyChar,
                        CHR_PROPS_NOTIFY,
                        refs.buttonState,
                        refs.buttonStateLen,
                        callbacks);
  ppSetupWritable(*refs.serviceEChar3, CHR_PROPS_WRITE, refs.shortValueLen, callbacks);
  ppSetupWritable(*refs.serviceEChar4, CHR_PROPS_WRITE, refs.shortValueLen, callbacks);
  refs.versionChar->setProperties(CHR_PROPS_READ);
  ppSetupReadableWithTrace(*refs.versionChar,
                           refs.versionStringBytes,
                           refs.versionStringLen,
                           callbacks);
  ppSetupWritable(*refs.configChar, CHR_PROPS_WRITE, refs.configStateLen, callbacks);
  refs.configChar->write(refs.configState, refs.configStateLen);

  refs.authService->begin();
  ppSetupNotifyVariable(*refs.authControl,
                        CHR_PROPS_WRITE_WO_RESP | CHR_PROPS_NOTIFY,
                        refs.authCtrlReply25,
                        refs.authCtrlReply25Len,
                        refs.mediumValueLen,
                        callbacks);
  refs.authControl->setWriteCallback(callbacks.serviceWriteCallback);
  ppSetupNotifyVariable(*refs.authBlob,
                        CHR_PROPS_WRITE_WO_RESP | CHR_PROPS_NOTIFY,
                        refs.authBlobVisibleValue,
                        refs.authBlobVisibleValueLen,
                        refs.authBlobMaxLen,
                        callbacks);
  refs.authBlob->setWriteCallback(callbacks.serviceWriteCallback);
}
