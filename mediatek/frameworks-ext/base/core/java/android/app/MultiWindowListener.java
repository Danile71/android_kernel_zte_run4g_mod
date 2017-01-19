
package android.app;

/**
 * Used for receiving notifications from the phoneWindow
 * 
 * @hide
 */
public abstract class MultiWindowListener {
            /**
         * M: BMW, Called when user press close button of floating action bar
         * @hide
         */
        public void onCloseFloatingWindow(){
        }

        /**
         * M: BMW, Called when user press max button of floating action bar
         * @hide
         */
        public void onMaxFloatingWindow(){
        }

        /**
         * M: BMW, Called when user press stick button of floating action bar
         * @hide
         */
        public void onStickFloatingWindow(){
        }
}
