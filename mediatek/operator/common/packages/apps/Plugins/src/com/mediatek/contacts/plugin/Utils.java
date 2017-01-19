package com.mediatek.contacts.plugin;
import com.mediatek.contacts.plugin.AASSNEOptions;
/**
 * M: Tools which not suitable to contains by SimUtils.java
 */
public class Utils {

    /**
     * M: The pre-defined command line of sne component calling
     */
    public static final String COMMD_FOR_SNE;
    static
    {
      if (com.mediatek.contacts.plugin.AASSNEOptions.SNE)
      {
         COMMD_FOR_SNE = "ExtensionForSNE";
      }
      else
      {
         COMMD_FOR_SNE = "NotSupported";
      }
    }

    /**
     * M: judge whether the cmd indicates the request of calling SNE Plug-in
     * @param cmd the command string
     * @return true if yes, false if no
     */
    public static final boolean isSNECalled(String cmd) {
        return (cmd.equals(COMMD_FOR_SNE) || cmd.contains(COMMD_FOR_SNE));
    }
}
