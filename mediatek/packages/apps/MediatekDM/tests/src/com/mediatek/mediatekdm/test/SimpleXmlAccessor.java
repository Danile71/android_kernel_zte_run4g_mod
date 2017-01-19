package com.mediatek.mediatekdm.test;

import android.util.Log;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

public class SimpleXmlAccessor {
    protected Document mDocument = null;
    private static final String TAG = "MDMTest/SimpleXmlAccessor";
    
    public void parse(InputStream is) {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            mDocument = builder.parse(is);
        } catch (ParserConfigurationException e) {
            throw new Error(e);
        } catch (SAXException e) {
            throw new Error(e);
        } catch (IOException e) {
            throw new Error(e);
        }
    }
    
    public void write(OutputStream os) {
        if (mDocument == null) {
            throw new Error("You should parse document first.");
        }
        TransformerFactory factory = TransformerFactory.newInstance();
        try {
            Transformer transformer = factory.newTransformer();
            transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
            DOMSource source = new DOMSource(mDocument);
            StreamResult result = new StreamResult(os);
            transformer.transform(source, result);
        } catch (TransformerConfigurationException e) {
            throw new Error(e);
        } catch (TransformerException e) {
            throw new Error(e);
        }
    }
    
    protected String getNodeContent(String path) {
        Log.d(TAG, "getNodeContent: " + path);
        if (mDocument == null) {
            throw new Error("You should parse document first.");
        }
        XPath xpath = XPathFactory.newInstance().newXPath();
        try {
            XPathExpression expression = xpath.compile(path);
            Node node = (Node) expression.evaluate(mDocument, XPathConstants.NODE);
            return node.getTextContent();
        } catch (XPathExpressionException e) {
            throw new Error(e);
        }
    }
    
    protected void setNodeContent(String path, String value){
        Log.d(TAG, "setNodeContent: " + path + ", " + value);
        if (mDocument == null) {
            throw new Error("You should parse document first.");
        }
        XPath xpath = XPathFactory.newInstance().newXPath();
        try {
            XPathExpression expression = xpath.compile(path);
            Node node = (Node) expression.evaluate(mDocument, XPathConstants.NODE);
            node.setTextContent(value);
        } catch (XPathExpressionException e) {
            throw new Error(e);
        }
    }
}
