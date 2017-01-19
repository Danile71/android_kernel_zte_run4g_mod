package com.mediatek.mediatekdm.wfhs.io;

import android.util.Log;

import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.MdmTree;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

public abstract class NodeIOHandlerInfo {

    private static final String TAG = "DM/Wifi.NodeIOHandlerInfo";

    public static LinkedList<NodeIOHandlerInfo> ioHandlerInfos;

    static {
        ioHandlerInfos = new LinkedList<NodeIOHandlerInfo>();
        ioHandlerInfos.add(new PreferredRoamingPartnerIOHandlerInfo());
        ioHandlerInfos.add(new CredentialIOHandlerInfo());
        ioHandlerInfos.add(new HomeSPIOHandlerInfo());
        ioHandlerInfos.add(new AAAServerTrustRootIOHandlerInfo());
    }

    public NodeIOHandlerInfo() {
        fetchInfos();
        for (int i = 0; i < preferNodes.length; i++) {
            mNodeToKey.put(preferNodes[i], preferKeys[i]);
        }
    }

    protected abstract void fetchInfos();
    
    public List<String> getRegisterUris(String hotspot) {
        List<String> uris = new LinkedList<String>();
        try {
                String[] xs = mTree.listChildren(String.format("%s/%s", hotspot, localUri));
            for (String x : xs) {
                    Log.i(TAG, "x = " + x);
                for (String node : preferNodes) {
                        String uri = String.format("%s/%s/%s/%s", hotspot, localUri, x, node);
                        Log.i(TAG, "uri = " + uri);
                    uris.add(uri);
                }
            }
        } catch (MdmException e) {
            e.printStackTrace();
        }
        return uris;
    }

    public String nodeToKey(String node) {
        for (String localNode : preferNodes) {
            if (node.endsWith(localNode)) {
                return mNodeToKey.get(localNode);
            }
        }
        return null;
    }

    /*
     * a node uri whose children leaves need to register node io handler
     */
    public String localUri;

    /*
     * node io handler's class name
     */
    public String handlerName;

    /*
     * the sessions node io handler should work
     */
    public Integer[] preferSessionIds;

    /*
     * leaves need to register node io handler
     */
    protected String[] preferNodes;

    /*
     * leaves need to register node io handler
     */
    protected String[] preferKeys;

    /*
     * use the keys to put the leaves value to the intent sending to the DM
     * client
     */
    protected HashMap<String, String> mNodeToKey = new HashMap<String, String>();
    
    protected MdmTree mTree = new MdmTree();
}
