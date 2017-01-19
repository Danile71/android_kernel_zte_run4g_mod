/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.passpoint;

import com.mediatek.kobjects.base64.Base64;

import java.io.ByteArrayInputStream;

import java.security.cert.X509Certificate;
import java.security.cert.CertificateFactory;

public class PasspointCertificateTest {
    private static String ID_H = "MIIFxDCCBKygAwIBAgICEDgwDQYJKoZIhvcNAQELBQAwgZIxCzAJBgNVBAYTAlVT"+
                                "MRMwEQYDVQQIDApDYWxpZm9ybmlhMRQwEgYDVQQHDAtTYW50YSBDbGFyYTEXMBUG"+
                                "A1UECgwOV2ktRmkgQWxsaWFuY2UxHTAbBgNVBAMMFFdGQSBSb290IENlcnRpZmlj"+
                                "YXRlMSAwHgYJKoZIhvcNAQkBFhFzdXBwb3J0QHdpLWZpLm9yZzAeFw0xMzA1MTAy"+
                                "MTEzMDdaFw0yMzA1MDgyMTEzMDdaMIGGMQswCQYDVQQGEwJVUzETMBEGA1UECAwK"+
                                "Q2FsaWZvcm5pYTEXMBUGA1UECgwOV2ktRmkgQWxsaWFuY2UxJzAlBgNVBAMMHk9T"+
                                "VSBTZXJ2ZXIgQ2VydGlmaWNhdGUgUktTIElESDEgMB4GCSqGSIb3DQEJARYRc3Vw"+
                                "cG9ydEB3aS1maS5vcmcwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDx"+
                                "SV5E4NTFWjffIYBB/M5qRPy0axOFU1VVVQoyAPJdLMG5qAI3/FxT/O098AV3MvsD"+
                                "YNQHj7KPnLTYHl7a0Wz7EFttWZF6rjZTYa/g75kLN7xM3RQ+k9KZxnaQpeoJeAVW"+
                                "AJAhaFrPsG3L3gjJWD5+jOHhdpA36ckhFD9CBS7hedgPOpSQ3B7riBErMQjxqTBE"+
                                "Viy3VvvCLgOPpuwDa8gEYGwByWMxgo/GBUnwii3o3eAoEcusNX0IbzOHsE7qRPh/"+
                                "rvOOVbSSWr0R6/xSWYDfw6UIEA9zbeTJ8LRLW/tIR06TGRJTZlW9AAsJTjgVbrDP"+
                                "DPYErAPmLsaTb5nndeQZAgMBAAGjggIsMIICKDAPBgNVHRMBAf8EBTADAgEAMAsG"+
                                "A1UdDwQEAwIF4DCB9QYDVR0RBIHtMIHqgixzdWJzY3JpcHRpb24tc2VydmVyLnIy"+
                                "LXRlc3RiZWQtcmtzLndpLWZpLm9yZ4ImcG9saWN5LXNlcnZlci5yMi10ZXN0YmVk"+
                                "LXJrcy53aS1maS5vcmeCK3JlbWVkaWF0aW9uLXNlcnZlci5yMi10ZXN0YmVkLXJr"+
                                "cy53aS1maS5vcmeCI29zdS1zZXJ2ZXIucjItdGVzdGJlZC1ya3Mud2ktZmkub3Jn"+
                                "oB8GCysGAQQBgr5oAQEBoBAMDkVOR0V4YW1wbGUgT1NVoB8GCysGAQQBgr5oAQEB"+
                                "oBAMDktPUkV4YW1wbGUgT1NVMBYGA1UdJQEB/wQMMAoGCCsGAQUFBwMBMB0GA1Ud"+
                                "DgQWBBR/9gFwQAUKYqrxCrQBTsMmitUdjjAfBgNVHSMEGDAWgBQLA8I+VKIovT5J"+
                                "3nLxX46rDpdngjCBtwYIKwYBBQUHAQwEgaowgaeggaQwgaGggZ4wgZswgZgwgZUW"+
                                "CWltYWdlL3BuZzBRME8wCwYJYIZIAWUDBAIBBEAwMTM5NjI3NjcyM2Y4ZjNkOGRk"+
                                "YzdmZDJiMzI5NjVhM2MyYzQ2Mjg0NzYyYWQxNDQ2YWY0YjNmMmNmMTYzMzE4MDUW"+
                                "M2h0dHA6Ly93d3cucjItdGVzdGJlZC53aS1maS5vcmcvMTM1NzE2MTQ3NV93aWZp"+
                                "LnBuZzANBgkqhkiG9w0BAQsFAAOCAQEAYpriqDgHR3XuzTq4QqRW+HiDmD92+fM1"+
                                "Yyr6LcHb5IoybMFv5mvTR+AMF2WHsoq/BPXUITLc8BUsagVjsPovKWgfWkH9+F4H"+
                                "GP4szqi1QbavrewmyjPwtL8dA03KX+SC6meJZvFtd+02ghG+W/zTmHdfx/Dfq62E"+
                                "j/pamK5upmU3nn7kqIASObVSxV5JPUurxFrCtfhs8MfHbbMD/RppnsvzD4yoxb0I"+
                                "tMQDJxv0TShi25UZTN+5uFtjsbyIA5/NS+SMudInZx/lijBTbzwiYkfn9F7urFXC"+
                                "j+ZEclzM7zADTFv7s/5gyiXqjpZor7yfazF8jZdgZ5VmRkAiv1vYQA==";

    private static String ID_I = "MIIF0zCCBLugAwIBAgICED0wDQYJKoZIhvcNAQELBQAwgZIxCzAJBgNVBAYTAlVT"+
                                "MRMwEQYDVQQIDApDYWxpZm9ybmlhMRQwEgYDVQQHDAtTYW50YSBDbGFyYTEXMBUG"+
                                "A1UECgwOV2ktRmkgQWxsaWFuY2UxHTAbBgNVBAMMFFdGQSBSb290IENlcnRpZmlj"+
                                "YXRlMSAwHgYJKoZIhvcNAQkBFhFzdXBwb3J0QHdpLWZpLm9yZzAeFw0xMzA1MTAy"+
                                "MTI2MjlaFw0yMzA1MDgyMTI2MjlaMIGGMQswCQYDVQQGEwJVUzETMBEGA1UECAwK"+
                                "Q2FsaWZvcm5pYTEXMBUGA1UECgwOV2ktRmkgQWxsaWFuY2UxJzAlBgNVBAMMHk9T"+
                                "VSBTZXJ2ZXIgQ2VydGlmaWNhdGUgUktTIElESTEgMB4GCSqGSIb3DQEJARYRc3Vw"+
                                "cG9ydEB3aS1maS5vcmcwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCe"+
                                "KXkAJVJah04+mUqWZmKXY0QaSVOyY2Xc0X4iN9vTu9q6yvneoFM/3PjU5Dkq0j/1"+
                                "gFmunEqMFn1rNZnN1Q7sg//n2yEb4k/zFrHLUsOPNdsPeU7DJraK/546C6b8H2ON"+
                                "KqA6+UmOOJVLrhpBxfMqW6OjawX0YvrzWGrD6x9uTLgHtq5XfRwgEmGyKg0+QCFG"+
                                "KSvVZ9P5YE9ob/xNju8JC/b3dCiupb7HnRdeg2U5FQ915kfaxEZkBHbV57i70IPc"+
                                "biz9kxOMPaQNfEgy9f3tm/FXkBcn/5nBTqsAS3vAcvuOT/V0c0BfO3GJTDjTwn9R"+
                                "rZCKS5g1fHHm21alzxSpAgMBAAGjggI7MIICNzAPBgNVHRMBAf8EBTADAgEAMAsG"+
                                "A1UdDwQEAwIF4DCCAQMGA1UdEQSB+zCB+IIsc3Vic2NyaXB0aW9uLXNlcnZlci5y"+
                                "Mi10ZXN0YmVkLXJrcy53aS1maS5vcmeCJnBvbGljeS1zZXJ2ZXIucjItdGVzdGJl"+
                                "ZC1ya3Mud2ktZmkub3JngityZW1lZGlhdGlvbi1zZXJ2ZXIucjItdGVzdGJlZC1y"+
                                "a3Mud2ktZmkub3JngiNvc3Utc2VydmVyLnIyLXRlc3RiZWQtcmtzLndpLWZpLm9y"+
                                "Z6AmBgsrBgEEAYK+aAEBAaAXDBVGSU5XaS1GaSBBbGxpYW5jZSBPU1WgJgYLKwYB"+
                                "BAGCvmgBAQGgFwwVQ0hJV2ktRmkgQWxsaWFuY2UgT1NVMBYGA1UdJQEB/wQMMAoG"+
                                "CCsGAQUFBwMBMB0GA1UdDgQWBBQHlAXuIDlihKIJ/se2I7X0C5ciRDAfBgNVHSME"+
                                "GDAWgBQLA8I+VKIovT5J3nLxX46rDpdngjCBtwYIKwYBBQUHAQwEgaowgaeggaQw"+
                                "gaGggZ4wgZswgZgwgZUWCWltYWdlL3BuZzBRME8wCwYJYIZIAWUDBAIBBEAwMTM5"+
                                "NjI3NjcyM2Y4ZjNkOGRkYzdmZDJiMzI5NjVhM2MyYzQ2Mjg0NzYyYWQxNDQ2YWY0"+
                                "YjNmMmNmMTYzMzE4MDUWM2h0dHA6Ly93d3cucjItdGVzdGJlZC53aS1maS5vcmcv"+
                                "MTM1NzE2MTQ3NV93aWZpLnBuZzANBgkqhkiG9w0BAQsFAAOCAQEAuRrN/Oy5UktR"+
                                "1FVKa+90L744XQ4pWFyYCR0u7aF1psWS7Xo51t0jXahqQygzNbHhj2DIYXN8N9da"+
                                "1SG7IoVp6OgANGzAoHiQpacIETEn1JYgO90DejR8ttUAXzdS0UL+Dt5xsHF3w3eH"+
                                "xdtNIPQfdJh8+jg47TGvoNfyMzxn/M/RNbDiWg+HR6gImGQsNjeCv+tFG0TZRq1T"+
                                "eRp3NjvvxY0cZI8L8LGUkmP8PSGc4y4MS7k+B4tfBJ7T2VsFxwhRFr/zIgUk1XXI"+
                                "d7nxOE235YDGgIYO3tQSJ1hC1cXtdFcRnmRDdHBsfMdB0JZ/550iniCKInmHYNZf"+
                                "fseIU8W+vQ==";

    private static String ID_J = "MIIF0zCCBLugAwIBAgICEEAwDQYJKoZIhvcNAQELBQAwgZIxCzAJBgNVBAYTAlVT"+
                                "MRMwEQYDVQQIDApDYWxpZm9ybmlhMRQwEgYDVQQHDAtTYW50YSBDbGFyYTEXMBUG"+
                                "A1UECgwOV2ktRmkgQWxsaWFuY2UxHTAbBgNVBAMMFFdGQSBSb290IENlcnRpZmlj"+
                                "YXRlMSAwHgYJKoZIhvcNAQkBFhFzdXBwb3J0QHdpLWZpLm9yZzAeFw0xMzA1MTAy"+
                                "MTM0NTRaFw0yMzA1MDgyMTM0NTRaMIGGMQswCQYDVQQGEwJVUzETMBEGA1UECAwK"+
                                "Q2FsaWZvcm5pYTEXMBUGA1UECgwOV2ktRmkgQWxsaWFuY2UxJzAlBgNVBAMMHk9T"+
                                "VSBTZXJ2ZXIgQ2VydGlmaWNhdGUgUktTIElESjEgMB4GCSqGSIb3DQEJARYRc3Vw"+
                                "cG9ydEB3aS1maS5vcmcwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC3"+
                                "LM0x9jOdtlPSwA2WezZ+L3fB/jwVR8LS1oa2qMptt/AgJ5/DnqMU8epzJr5nPGu8"+
                                "AvCZG4/FUNm1wEaCuULudUjLGwMkEqeYlmUpIZtxZ2X3yRRzV7NvbIMH8Y20QDxC"+
                                "kmtbeQecYx6RCYvDdTULNY712QtpL3sAxWjnO/8F6fknobv7oVksyJ21EYacF0mJ"+
                                "gNAxyuSO1dReFtD0YpFj7nNbxHx+5F/J5yhnMMvZc1orfVMkdYQjt/3o8aAxencB"+
                                "TPUCh41fBXAGthLkoh2eahlis2XZ5TJQD8VjUlLURFTpJEVHaVkfAP4nExJBd6Ut"+
                                "qe59GyRCC8T5614MZZuLAgMBAAGjggI7MIICNzAPBgNVHRMBAf8EBTADAgEAMAsG"+
                                "A1UdDwQEAwIF4DCCAQMGA1UdEQSB+zCB+IIsc3Vic2NyaXB0aW9uLXNlcnZlci5y"+
                                "Mi10ZXN0YmVkLXJrcy53aS1maS5vcmeCJnBvbGljeS1zZXJ2ZXIucjItdGVzdGJl"+
                                "ZC1ya3Mud2ktZmkub3JngityZW1lZGlhdGlvbi1zZXJ2ZXIucjItdGVzdGJlZC1y"+
                                "a3Mud2ktZmkub3JngiNvc3Utc2VydmVyLnIyLXRlc3RiZWQtcmtzLndpLWZpLm9y"+
                                "Z6AmBgsrBgEEAYK+aAEBAaAXDBVFTkdXaS1GaSBBbGxpYW5jZSBPU1WgJgYLKwYB"+
                                "BAGCvmgBAQGgFwwVS09SV2ktRmkgQWxsaWFuY2UgT1NVMBYGA1UdJQEB/wQMMAoG"+
                                "CCsGAQUFBwMBMB0GA1UdDgQWBBT/KnzwpXVkbffVCKHAF8DeMB+fgDAfBgNVHSME"+
                                "GDAWgBQLA8I+VKIovT5J3nLxX46rDpdngjCBtwYIKwYBBQUHAQwEgaowgaeggaQw"+
                                "gaGggZ4wgZswgZgwgZUWCWltYWdlL3BuZzBRME8wCwYJYIZIAWUDBAIBBEAwMTM5"+
                                "NjI3NjcyM2Y4ZjNkOGRkYzdmZDJiMzI5NjVhM2MyYzQ2Mjg0NzYyYWQxNDQ2YWY0"+
                                "YjNmMmNmMTYzMzE5MDUWM2h0dHA6Ly93d3cucjItdGVzdGJlZC53aS1maS5vcmcv"+
                                "MTM1NzE2MTQ3NV93aWZpLnBuZzANBgkqhkiG9w0BAQsFAAOCAQEAJxTGpIFTQXiA"+
                                "6uoHXb8/kUKGLIaQrhh7e5MMEKrzph0Z3anm3UDrMwcfFTM7xKaoUMzDUmD3BHgT"+
                                "mTJgthUQMrm9mihuTtqGxCernG65GEHLq1V7wpq+IpexOPG9Ga++z2A7mxNtg6Vl"+
                                "BoZMPec7BSUZQI6k7VrcArvSksnZNXCmBgsK+QZKixdwBIgk06HiHBStnRQguI8B"+
                                "DGT/JTZDfh5fhopF7Np35W/YY3Cpw2SNhHKOGEaBfQUDg8Oxfioois1k1i9bVKkg"+
                                "GgbgOXEIOkYIWheF7cKFsX6b1otO7MfyZt/vQ9UP/D/bhihVaa9j1TJnQmugOA1S"+
                                "+hiI3V/LPA==";

    private static String ID_K = "MIIF0DCCBLigAwIBAgICEAgwDQYJKoZIhvcNAQELBQAwgY8xCzAJBgNVBAYTAlVT"+
                                "MRMwEQYDVQQIDApDYWxpZm9ybmlhMRQwEgYDVQQHDAtTYW50YSBDbGFyYTEXMBUG"+
                                "A1UECgwOV2ktRmkgQWxsaWFuY2UxGjAYBgNVBAMMEVVudHJ1c3RlZCBSb290IENB"+
                                "MSAwHgYJKoZIhvcNAQkBFhFzdXBwb3J0QHdpLWZpLm9yZzAeFw0xMzA1MTAyMjAw"+
                                "MTRaFw0yMzA1MDgyMjAwMTRaMIGGMQswCQYDVQQGEwJVUzETMBEGA1UECAwKQ2Fs"+
                                "aWZvcm5pYTEXMBUGA1UECgwOV2ktRmkgQWxsaWFuY2UxJzAlBgNVBAMMHk9TVSBT"+
                                "ZXJ2ZXIgQ2VydGlmaWNhdGUgUktTIElESzEgMB4GCSqGSIb3DQEJARYRc3VwcG9y"+
                                "dEB3aS1maS5vcmcwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC1b50D"+
                                "yE5lhdTd8w0psqjqMm3s7c+7aGDTI+N/LbRhzH56TfJIRepBY9M1KLNhKY5bpvgf"+
                                "GVuY/Vvulv7SwsUKU9vIeu8FSo8+lNeZTb0MSEzqg4ee66ngYXg9IOF3W50FytP6"+
                                "zFuoA8szF5lXkfVh632s9C7/+xkn6hEQdXPUr/fLojcBvj98OUvXtFV137LUqIxc"+
                                "mIBizybxSrD1AcAnkAg76QZgkJwEhSsMmVPHD566LAIiluvn8fa47mzK5UASLUSE"+
                                "AEuYGL2saqBwHsROfR93HLGkiq8/09Po1RVw4EIpEiiekHjSK1dHBJDmDpfDUWsW"+
                                "BCLNMsgCqyS/Bk+JAgMBAAGjggI7MIICNzAPBgNVHRMBAf8EBTADAgEAMAsGA1Ud"+
                                "DwQEAwIF4DCCAQMGA1UdEQSB+zCB+IIsc3Vic2NyaXB0aW9uLXNlcnZlci5yMi10"+
                                "ZXN0YmVkLXJrcy53aS1maS5vcmeCJnBvbGljeS1zZXJ2ZXIucjItdGVzdGJlZC1y"+
                                "a3Mud2ktZmkub3JngityZW1lZGlhdGlvbi1zZXJ2ZXIucjItdGVzdGJlZC1ya3Mu"+
                                "d2ktZmkub3JngiNvc3Utc2VydmVyLnIyLXRlc3RiZWQtcmtzLndpLWZpLm9yZ6Am"+
                                "BgsrBgEEAYK+aAEBAaAXDBVFTkdXaS1GaSBBbGxpYW5jZSBPU1WgJgYLKwYBBAGC"+
                                "vmgBAQGgFwwVS09SV2ktRmkgQWxsaWFuY2UgT1NVMBYGA1UdJQEB/wQMMAoGCCsG"+
                                "AQUFBwMBMB0GA1UdDgQWBBTdsTjXUb+VyJNzmhp1qj91MXheOzAfBgNVHSMEGDAW"+
                                "gBSZgzWpf18LKUYzWNFM/E7pKB4T7TCBtwYIKwYBBQUHAQwEgaowgaeggaQwgaGg"+
                                "gZ4wgZswgZgwgZUWCWltYWdlL3BuZzBRME8wCwYJYIZIAWUDBAIBBEAwMTM5NjI3"+
                                "NjcyM2Y4ZjNkOGRkYzdmZDJiMzI5NjVhM2MyYzQ2Mjg0NzYyYWQxNDQ2YWY0YjNm"+
                                "MmNmMTYzMzE4MDUWM2h0dHA6Ly93d3cucjItdGVzdGJlZC53aS1maS5vcmcvMTM1"+
                                "NzE2MTQ3NV93aWZpLnBuZzANBgkqhkiG9w0BAQsFAAOCAQEAA0v4FW1z73pt3TNj"+
                                "u1b9is7Xc3UdLPdMsTsd/UcrDdGHgNzvYddWFj/cac+z2fOdmgyn29d+iexJDDP/"+
                                "gr2xIjFDlM7JgduhC/2JtiseWkpwzTJxggcGSItTOD8gP4T17Dz0yCxKhJQ76RjQ"+
                                "A0AsZz0tDrHsyu3YTZ9ifw8BnpIEK8MOhrYYjt2Zua0DaYj0A61bePQZawU2zpMm"+
                                "qGnUEcOjE1xYNwDTejThzeVIIfICvAFNped7DWdjjx6vWDGLo0PUbKvBYjFXroHQ"+
                                "Bj3lNqCpTUiGdIuaEMVCb1ssFKuV7JIzWDih37LZgyxKTD3wbur8k+hlooUCQwPE"+
                                "v3chZQ==";
    
    public static X509Certificate getID_H(){
        return convertPEMtoX509(ID_H);
    }

    public static X509Certificate getID_I(){
        return convertPEMtoX509(ID_I);
    }

    public static X509Certificate getID_J(){
        return convertPEMtoX509(ID_J);
    }

    public static X509Certificate getID_K(){
        return convertPEMtoX509(ID_K);
    }
    
    private static X509Certificate convertPEMtoX509(String pem){
        try{
            byte[] cert = Base64.decode(pem);
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            X509Certificate x509Cert = (X509Certificate) cf
                .generateCertificate(new ByteArrayInputStream(cert));
            
            return x509Cert;
        }catch(Exception e){
            e.printStackTrace();
        }

        return null;
    }
}


