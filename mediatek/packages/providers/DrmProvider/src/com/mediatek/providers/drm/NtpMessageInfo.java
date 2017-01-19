package com.mediatek.providers.drm;

import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public class NtpMessageInfo {
    /** *//**
     * It indicates that the time last minute is OK or not.
     * 0 means no warning,
     * 1 means last minute has 61 seconds,
     * 2 means last minute has 59 seconds,
     * 3 means alarm condition (clock not synchronized).
     */
    public byte mLeapTimeFlag = 0;

    /** *//**
     * It indicates the version of NTP/SNTP.
     * 3 means version 3, which is only for IPv4,
     * 4 means version 4, which is for IPv4,IPv6 and OSI,
     * If necessary to distinguish between IPv4, IPv6 and OSI,
     * need to check the encapsulating context.
     */
    public byte mIPVersion = 3;

    /** *//**
     * It indicates the mode.
     * 
     * 0 means reserved,
     * 1 means symmetric active,
     * 2 means symmetric passive,
     * 3 means client,
     * 4 means server,
     * 5 means broadcast,
     * 6 means reserved for NTP control message,
     * 7 means reserved for private use.
     * 
     * The client will set mode to 3(client) in request, and server will set mode 
     * to 4(server) if this is in unicast and any cast modes.
     * Server will set mode to 5(broadcast) if this is in multicast mode.
     * 
     */
    public byte mNTPMode = 0;

    /** *//**
     * It indicates stratum level of local clock.
     * 
     * 0 means unspecified or unavailable,
     * 1 means primary reference (e.g., radio clock),
     * 2-15 means secondary reference (via NTP or SNTP),
     * 16-255 means reserved.
     * 
     */
    public short mClockLevel = 0;

    /** *//**
     * It indicates the maximum time will be spent between successive messages, which is 
     * the nearest power of two in seconds. Value will be from 4(2^4=16s) to 
     * 14(2^14=16284s), most value will be between 6(2^6=64s) and 10(2^10=1024s).
     * 
     */
    public byte mMessageInterval = 0;

    /** *//**
     * It indicates the local clock precision, it is the nearest power of two
     * in seconds. Value will be from -6(main-frequency clock) to -20(microsecond clocks)
     * most of time.
     * 
     */
    public byte mClockPrecision = 0;

    /** *//**
     * It indicates the delay of total round trip to the primary reference
     * source in seconds. This value will be positive and negative, which depends
     * on frequency offsets and relative time. Its range will be from negative of
     * few milliseconds to positive of several hundred milliseconds. 
     * 
     */
    public double mRoundTripDelay = 0;

    /** *//**
     * This value indicates the nominal error relative to the primary reference
     * source, in seconds. The values that normally appear in this field range
     * from 0 to several hundred milliseconds.
     */
    public double mDevision = 0;

    /** *//**
     * It is a 4-byte array to identify the particular reference source.
     * For NTP Version 3 secondary server, It is 32-bit IPV4 address of referenced source.
     * For NTP Version 4 secondary server, It is low order 32-bit of the latest transmit
     * timestamp of the referenced source.
     * NTP primary servers should set the value to identify the external referenced source
     * according to following list. If it is in the list, the associated value should be used.
     * If it is not in the list, the value can be created as appropriate.
     * 
     * List of Code External Reference Source as below:
     * LOCL is that when a subnet without external means of synchronization, it will use
     * a uncalibrated local clock as a primary reference.
     * PPS is that it uses national standards to individually calibrate for atomic clock
     * or other pulse-per-second source.
     * GOES is Geostationary Orbit Environment Satellite.
     * GPS is Global Positioning Service.
     * OMEG is OMEGA radionavigation system.
     * LORC is LORAN-C radionavigation system.
     * CHU is Ottawa (Canada) Radio 3330, 7335, 14670 kHz.
     * WWVH is Kaui Hawaii (US) Radio 2.5, 5, 10, 15 MHz.
     * WWVB is Boulder (US) Radio 60 kHz. 
     * WWV is Ft. Collins (US) Radio 2.5, 5, 10, 15, 20 MHz.
     * MSF is Rugby (UK) Radio 60 kHz. 
     * DCF is Mainflingen (Germany) Radio 77.5 kHz. 
     * TDF is Allouis (France) Radio 164 kHz.
     * PTB is PTB (Germany) modem service.
     * USNO is USNO modem service. 
     * ACTS is NIST dialup modem service.
     * 
     */
    public byte[] mReferencedIdentifier = { 0, 0, 0, 0 };

    /** *//**
     * It is the time that the local clock was last set or corrected, using seconds
     * to record since 00:00 1-Jan-1900.
     */
    public double mReferencedClockTime = 0;

    /** *//**
     * It is the time that the request was sent from the client to the server,using seconds
     * to record since 00:00 1-Jan-1900.
     */
    public double mSendRequestTime = 0;

    /** *//**
     * It is the time that the request was received at the server, using seconds
     * to record since 00:00 1-Jan-1900.
     */
    public double mReceivedRequestTime = 0;

    /** *//**
     * It is the time that the response was sent from the server to the client,using seconds
     * to record since 00:00 1-Jan-1900.
     */
    public double mSendResponseTime = 0;

    /** *//**
     * Constructor. Using an array of bytes as parameters to construct
     * a new NtpMessageInfo.
     * @param byteArray data packet byte array, visit RFC 2030 format for details.
     */
    public NtpMessageInfo(byte[] byteArray) {
        mLeapTimeFlag = (byte) ((byteArray[0] >> 6) & 0x3);
        mIPVersion = (byte) ((byteArray[0] >> 3) & 0x7);
        mNTPMode = (byte) (byteArray[0] & 0x7);
        mClockLevel = convertByteToShort(byteArray[1]);
        mMessageInterval = byteArray[2];
        mClockPrecision = byteArray[3];

        mRoundTripDelay = (byteArray[4] * 256.0)
                     + convertByteToShort(byteArray[5])
                     + (convertByteToShort(byteArray[6]) / 256.0)
                     + (convertByteToShort(byteArray[7]) / 65536.0);

        mDevision = (convertByteToShort(byteArray[8]) * 256.0)
               + convertByteToShort(byteArray[9])
               + (convertByteToShort(byteArray[10]) / 256.0)
               + (convertByteToShort(byteArray[11]) / 65536.0);

        mReferencedIdentifier[0] = byteArray[12];
        mReferencedIdentifier[1] = byteArray[13];
        mReferencedIdentifier[2] = byteArray[14];
        mReferencedIdentifier[3] = byteArray[15];

        mReferencedClockTime = unPackByteArrayToTime(byteArray, 16);
        mSendRequestTime = unPackByteArrayToTime(byteArray, 24);
        mReceivedRequestTime = unPackByteArrayToTime(byteArray, 32);
        mSendResponseTime = unPackByteArrayToTime(byteArray, 40);
    }

    /** *//**
     * Constructor. Using detail parameters to construct a new NtpMessageInfo.
     * It has not been used.
     */
    public NtpMessageInfo(byte leapTimeFlag, byte ipVersion, byte ntpMode, short clockLevel,
            byte messageInterval, byte clockPrecision, double roundTripDelay, double devision,
            byte[] referencedIdentifier, double referencedClockTime, double sendRequestTime,
            double receivedRequestTime, double sendResponseTime) {
        // ToDo: Validity checking before assignment
        this.mLeapTimeFlag = leapTimeFlag;
        this.mIPVersion = ipVersion;
        this.mNTPMode = ntpMode;
        this.mClockLevel = clockLevel;
        this.mMessageInterval = messageInterval;
        this.mClockPrecision = clockPrecision;
        this.mRoundTripDelay = roundTripDelay;
        this.mDevision = devision;
        this.mReferencedIdentifier = referencedIdentifier;
        this.mReferencedClockTime = referencedClockTime;
        this.mSendRequestTime = sendRequestTime;
        this.mReceivedRequestTime = receivedRequestTime;
        this.mSendResponseTime = sendResponseTime;
    }

    /** *//**
     * Constructor. If in client - server mode, construct a new NtpMessage without parameter.
     * This will use current time as transmit timestamp.
     * 
     */
    public NtpMessageInfo() {
        // Only need to update mode and send response time,
        // other value will use default value.
        this.mNTPMode = 3;
        this.mSendResponseTime = (System.currentTimeMillis() / 1000.0) + 2208988800.0;
    }

    /** *//**
     * This method will convert NtpMessageInfo data to data bytes.
     */
    public byte[] toByteArray() {
        // new array byte, values will reset to 0.
        byte[] tarByte = new byte[48];

        tarByte[0] = (byte) (mLeapTimeFlag << 6 | mIPVersion << 3 | mNTPMode);
        tarByte[1] = (byte) mClockLevel;
        tarByte[2] = (byte) mMessageInterval;
        tarByte[3] = (byte) mClockPrecision;

        // round trip delay is a signed 16.16-bit FP, but an int is 32-bits in java.
        int signedFP = (int) (mRoundTripDelay * 65536.0);
        tarByte[4] = (byte) ((signedFP >> 24) & 0xFF);
        tarByte[5] = (byte) ((signedFP >> 16) & 0xFF);
        tarByte[6] = (byte) ((signedFP >> 8) & 0xFF);
        tarByte[7] = (byte) (signedFP & 0xFF);

        // devision is an unsigned 16.16-bit FP, but there are no matched types in java,
        // so a long(64-bits) will be used.
        long unSignedFP = (long) (mDevision * 65536.0);
        tarByte[8] = (byte) ((unSignedFP >> 24) & 0xFF);
        tarByte[9] = (byte) ((unSignedFP >> 16) & 0xFF);
        tarByte[10] = (byte) ((unSignedFP >> 8) & 0xFF);
        tarByte[11] = (byte) (unSignedFP & 0xFF);

        tarByte[12] = mReferencedIdentifier[0];
        tarByte[13] = mReferencedIdentifier[1];
        tarByte[14] = mReferencedIdentifier[2];
        tarByte[15] = mReferencedIdentifier[3];

        packTimeToByteArray(tarByte, 16, mReferencedClockTime);
        packTimeToByteArray(tarByte, 24, mSendRequestTime);
        packTimeToByteArray(tarByte, 32, mReceivedRequestTime);
        packTimeToByteArray(tarByte, 40, mSendResponseTime);

        return tarByte;
    }

    /** *//**
     * This method will convert NtpMessageInfo to a string with particular style.
     */
    public String toString() {
        String preStr = new DecimalFormat("0.#E0").format(Math.pow(2, mClockPrecision));
        StringBuilder sb = new StringBuilder(31);
        sb.append("Leap time flg: ");
        sb.append(mLeapTimeFlag);
        sb.append(" ,IP Version: ");
        sb.append(mIPVersion);
        sb.append(" ,mNTPMode: ");
        sb.append(mNTPMode);
        sb.append(" ,mClockLevel: ");
        sb.append(mClockLevel);
        sb.append(" ,mMessageInterval: ");
        sb.append(mMessageInterval);
        sb.append(" ,mClockPrecision: ");
        sb.append(mClockPrecision);
        sb.append(" (");
        sb.append(preStr);
        sb.append(" seconds) ");
        sb.append(" ,mRoundTripDelay: ");
        sb.append(new DecimalFormat("0.00").format(mRoundTripDelay * 1000));
        sb.append(" ms ");
        sb.append(" ,mDevision: ");
        sb.append(new DecimalFormat("0.00").format(mDevision * 1000));
        sb.append(" ms ");
        sb.append(" ,mReferencedIdentifier: ");
        sb.append(convertIdentifierToString(mReferencedIdentifier, mClockLevel, mIPVersion));
        sb.append(" ,mReferencedClockTime: ");
        sb.append(mReferencedClockTime);
        sb.append(" ,mSendRequestTime: ");
        sb.append(mSendRequestTime);
        sb.append(" ,mReceivedRequestTime: ");
        sb.append(mReceivedRequestTime);
        sb.append(" ,mSendResponseTime: ");
        sb.append(mSendResponseTime);
        
        return sb.toString();
    }

    /** *//**
     * This method will convert an unsigned byte, then return a short to caller.
     * But Java assumes this byte is signed.
     * 
     */
    public static short convertByteToShort(byte unsignedByte) {
        if (0x80 == (unsignedByte & 0x80)) {
            return (short) (128 + (unsignedByte & 0x7f));
        } else {
            return (short) unsignedByte;
        }
    }

    /** *//**
     * This method will read 8 bytes from beginning of position, and using NTP
     * 64-bit timestamp format to convert it as a double.
     * 
     */
    public static double unPackByteArrayToTime(byte[] byteArray, int position) {
        double d = 0.0;

        for (int i = 0; i < 8; i++) {
            double base = Math.pow(2, (3 - i) * 8);
            d += convertByteToShort(byteArray[position + i]) * base;
        }

        return d;
    }

    /** *//**
     * Pack time to byte in specified position.
     * 
     */
    public static void packTimeToByteArray(byte[] byteArray, int position, double time) {
        // This will covert a double to a 64-bit in specified position.
        for (int i = 0; i < 8; i++) {
            // 2^24, 2^8, .., 2^-24, 2^-32
            double base = Math.pow(2, (3 - i) * 8);

            // Put convert byte to byte array
            byteArray[position + i] = (byte) (time / base);

            // Subtract converted value from remaining value.
            time = time - (double) (convertByteToShort(byteArray[position + i]) * base);
        }

        // In RFC 2030: It is suggested that using a random to fill the non-significant
        // low order bits of the timestamp for unbiased bitstring, this can avoid errors
        // of systematic roundoff and as a means of loop and replay detection.
        byteArray[7] = (byte) (Math.random() * 255.0);
    }

    /** *//**
     * This method will convert time(seconds since 00:00 1-Jan-1900) to a
     * formatted date/time string.
     * 
     */
    public static String converTimeToString(double time) {
        if (time == 0) {
            return "0";
        }

        // Utc is relative to 1970 in Java.
        // Timestamp is relative to 1900.
        double utcTime = time - (2208988800.0);

        // Convert it to milliseconds.
        long millisec = (long) (utcTime * 1000.0);

        // Define date/time string format.
        String dateStr = new SimpleDateFormat("dd-MMM-yyyy HH:mm:ss").format(new Date(millisec));

        double fractionTime = time - ((long) time);
        String fractionStr = new DecimalFormat(".000000").format(fractionTime);

        return dateStr + fractionStr;
    }

    /** *//**
     * This method will convert referenced identifier to string according to
     * the rules in RFC 2030.
     */
    public static String convertIdentifierToString(byte[] refIdentifier, short clockLevel, byte ipVersion) {
        // In RFC 2030: NTP version3 or NTP version 4 server, it is four-character ASCII
        // string, padded to 32 bits.
        if (clockLevel == 0 || clockLevel == 1) {
            return new String(refIdentifier);
        } else if (ipVersion == 3) {
            // If version 3, it is 32-bit ipv4 address.
            StringBuilder sb = new StringBuilder(7);
            sb.append(convertByteToShort(refIdentifier[0]));
            sb.append(".");
            sb.append(convertByteToShort(refIdentifier[1]));
            sb.append(".");
            sb.append(convertByteToShort(refIdentifier[2]));
            sb.append(".");
            sb.append(convertByteToShort(refIdentifier[3]));
            return sb.toString();
        } else if (ipVersion == 4) {
            // If version 4, it is low order 32-bit of the latest dispatch time.
            StringBuilder sb = new StringBuilder(5);
            sb.append("");
            sb.append(convertByteToShort(refIdentifier[0]) / 256.0);
            sb.append(convertByteToShort(refIdentifier[1]) / 65536.0);
            sb.append(convertByteToShort(refIdentifier[2]) / 16777216.0);
            sb.append(convertByteToShort(refIdentifier[3]) / 4294967296.0);
            return sb.toString();
        }

        return "";
    }
}
