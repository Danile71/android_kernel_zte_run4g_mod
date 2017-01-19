package com.mediatek.common.gba;

import com.mediatek.common.gba.NafSessionKey;

interface IGbaService {
    int getGbaSupported();
    boolean isGbaKeyExpired(String nafFqdn, in byte[] nafSecurProtocolId);
    NafSessionKey runGbaAuthentication(in String nafFqdn, in byte[] nafSecurProtocolId, boolean forceRun);
}