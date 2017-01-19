package com.discretix.drmactions.auxillary;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.MalformedURLException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import android.nfc.FormatException;


/**
 * Domain initiator parser. Extracts domain controller, custom data, account and service IDs form the initiator
 */
public class DomainInitiatorParser {
	/**
	 * domain controller
	 */
	public String domainController = null;
	
	/**
	 * service ID
	 */
	public String serviceId        = null;
	
	/**
	 * account ID
	 */
	public String accountId        = null;
	
	/**
	 * custom data
	 */
	public String customData       = null;
	
	/**
	 * convert empty strings to null
	 * trim string
	 * @param inputString input string
	 * @return trimmed string or null
	 */
	private String interpolateString(String inputString){
		//in case attribute is not found empty string returned
		if (inputString.isEmpty()) return null;
		
		//remove redundant spaces
		return inputString.trim();
	}
	/**
	 * Create instance DomainInitiatorParser and parse the initiator
	 * @param fileName initiator file path
	 * @throws FormatException in case initiator is malformed or non readable
	 */
	public DomainInitiatorParser(String fileName) throws FormatException{
		Document xmlDoc = null;
		try {
			DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			DocumentBuilder db;
			db = dbf.newDocumentBuilder();
			xmlDoc = db.parse(new InputSource(new FileInputStream(fileName)));
			xmlDoc.getDocumentElement().normalize();
		} catch (MalformedURLException e) {
			throw new FormatException("XML path is invalid");
		} catch (ParserConfigurationException e) {
			throw new FormatException("Failed to build XML document: " + e.getMessage());
		} catch (SAXException e) {
			throw new FormatException("Failed to parse XML document: " + e.getMessage());
		} catch (IOException e) {
			throw new FormatException("Failed to download XML document: " + e.getMessage());
		}
		XPath xpath = XPathFactory.newInstance().newXPath();
		try {
			domainController = interpolateString((String)xpath.evaluate("//DomainController/text()", xmlDoc, XPathConstants.STRING));
			serviceId		 = interpolateString((String)xpath.evaluate("//DS_ID/text()"           , xmlDoc, XPathConstants.STRING));
			accountId        = interpolateString((String)xpath.evaluate("//AccountID/text()"       , xmlDoc, XPathConstants.STRING));
			customData       = interpolateString((String)xpath.evaluate("//CustomData/text()"      , xmlDoc, XPathConstants.STRING));
			
		} catch (XPathExpressionException e) {
			e.printStackTrace();
			throw new FormatException("Failed to evaluate XPATH expression");
		}
	}
}
