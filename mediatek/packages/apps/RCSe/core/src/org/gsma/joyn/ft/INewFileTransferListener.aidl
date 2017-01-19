package org.gsma.joyn.ft;

/**
 * Callback method for new file transfer invitations and delivery reports
 */
interface INewFileTransferListener {
	void onNewFileTransfer(in String transferId);
	
	void onReportFileDelivered(String transferId);
	
	void onReportFileDisplayed(String transferId);
	
	void onFileDeliveredReport(String transferId,String contact);
	
	void onFileDisplayedReport(String transferId,String contact);
}