package com.mediatek.passpoint;

import android.util.Log;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class FileOperationUtil{
    private static final String TAG = "FileOperationUtil";

    public static String Read(String file){
        String text = null;

        try {         
            FileInputStream fis = new FileInputStream(file);  
            BufferedInputStream bis = new BufferedInputStream(fis);  
            DataInputStream dis = new DataInputStream(fis);  

            String buf;
            while((buf = dis.readLine()) != null){
                Log.d(TAG,"Read:" + buf);
                text += buf;
            } 
   
        }catch(IOException e) { 
            e.printStackTrace(); 
        }

        return text;                     
    }

    public static byte[] Read(File file) throws IOException{

        ByteArrayOutputStream ous = null;
        try {
            byte[] buffer = new byte[4096];
            ous = new ByteArrayOutputStream();
            InputStream ios = new FileInputStream(file);
            int read = 0;
            while ( (read = ios.read(buffer)) != -1 ) {
                ous.write(buffer, 0, read);
            }
        } finally { 
            try {
                 if ( ous != null ) 
                     ous.close();
            } catch ( IOException e) {
            }
        }
        return ous.toByteArray();
    }

    public static void writeBytesToFile(byte[] bytes, String filePath){
        try{            
            FileOutputStream fos;

            fos = new FileOutputStream(filePath);
            fos.write(bytes);
            fos.close();            
        }catch(Exception e){
            e.printStackTrace();
        }

    }    
}
