package com.mediatek.mediatekdm.wfhs;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

public class DmXMLStringParser {
    private Document mDocument = null;
    private boolean mIsParseSucceed = false;
    private String mFileName;

    public DmXMLStringParser(String xmlString) {
        System.out.printf("DmXMLParser parse xml string " + xmlString + "begin");
        mIsParseSucceed = parse(xmlString);
        System.out.printf("DmXMLParser parse xml string" + xmlString + "done");
    }

    boolean parse(String xmlString) {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            mDocument = builder.parse(new InputSource( new StringReader(xmlString)));
        } catch (SAXException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public String getValByTagName(String tagName) {
        NodeList dmValueNodeList = mDocument.getElementsByTagName(tagName);
        if (dmValueNodeList == null) {
            throw new IllegalArgumentException("getElementsByTagName failed for " + tagName);
        }
        Node dmValueNode = dmValueNodeList.item(0);
        return dmValueNode.getTextContent();
    }

    public void setValByTagName(String tagName, String value) {
        NodeList dmValueNodeList = mDocument.getElementsByTagName(tagName);
        if (dmValueNodeList == null) {
            throw new IllegalArgumentException("getElementsByTagName failed for " + tagName);
        }

        Node dmValueNode = dmValueNodeList.item(0);
        dmValueNode.setTextContent(value);
    }

    // If modify the content of document, should call this API to write back
    public void writeBack() {
        try {
            TransformerFactory tf = TransformerFactory.newInstance();
            Transformer transformer = tf.newTransformer();
            DOMSource source = new DOMSource(mDocument);
            StreamResult result = new StreamResult(new File(mFileName));
            transformer.transform(source, result);
        } catch (TransformerException e) {
            e.printStackTrace();
        }
    }

    public void getChildNode(Node node, List<Node> nodeList, String... nodeName) {
        if (!mIsParseSucceed) {
            return;
        }

        if (nodeName.length == 0) {
            getChildNodeList(node, null, nodeList, false);
        } else {
            getChildNodeList(node, nodeName[0], nodeList, false);
        }
        return;
    }

    public void getChildNode(List<Node> nodeList, String... nodeName) {
        if (!mIsParseSucceed) {
            return;
        }

        if (nodeName.length == 0) {
            getChildNodeList(mDocument, null, nodeList, false);
        } else {
            getChildNodeList(mDocument, nodeName[0], nodeList, false);
        }
        return;
    }

    public void getChildNodeAtLevel(Node node, List<Node> nodeList, int level, String... nodeName) {
        if (!mIsParseSucceed) {
            return;
        }

        if (nodeName.length == 0) {
            getChildNodeList(node, null, nodeList, false, level);
        } else {
            getChildNodeList(node, nodeName[0], nodeList, false, level);
        }
        return;
    }

    public void getChildNodeAtLevel(List<Node> nodeList, int level, String... nodeName) {
        if (!mIsParseSucceed) {
            return;
        }

        if (nodeName.length == 0) {
            getChildNodeList(mDocument, null, nodeList, false, level);
        } else {
            getChildNodeList(mDocument, nodeName[0], nodeList, false, level);
        }
        return;
    }

    public void getLeafNode(Node node, List<Node> nodeList, String... nodeName) {
        if (!mIsParseSucceed) {
            return;
        }

        if (nodeName.length == 0) {
            getChildNodeList(node, null, nodeList, true);
        } else {
            getChildNodeList(node, nodeName[0], nodeList, true);
        }
        return;
    }

    public void getLeafNode(List<Node> nodeList, String... nodeName) {
        if (!mIsParseSucceed) {
            return;
        }

        if (nodeName.length == 0) {
            getChildNodeList(mDocument, null, nodeList, true);
        } else {
            getChildNodeList(mDocument, nodeName[0], nodeList, true);
        }
        return;
    }

    /**
     * get child node of specified node Parameters: node - root node; NodeName -
     * find the node with NodeName tag, if NodeName is null, all child node will
     * be added nodeList list to which all node found will be added; level - if
     * the first element, level[0], exist, the search will be done by level,
     * otherwise in pre-order;
     */

    protected void getChildNodeList(Node node, String nodeName, List<Node> nodeList,
            boolean onlyleaf, int... level) {
        if (node == null && nodeList == null) {
            return;
        }
        if (((((level.length == 0) && node.getNodeName().equalsIgnoreCase(nodeName)) && nodeName != null) && !onlyleaf)
                || (level.length != 0 && level[0] == 0)) {
            if (node.getNodeType() == Node.ELEMENT_NODE
                    && (nodeName == null || node.getNodeName().equalsIgnoreCase(nodeName))) {
                nodeList.add(node);
            }
            if ((level.length != 0 && level[0] == 0)) {
                return;
            }
        }
        // else { ???
        Node childNode = node.getFirstChild();
        Node siblingNode = childNode;
        if (childNode != null) {

            if (childNode.getNodeType() == Node.ELEMENT_NODE) {
                if (nodeName == null && level.length == 0) {
                    nodeList.add(childNode);
                }
                if (level.length == 0) {
                    getChildNodeList(siblingNode, nodeName, nodeList, onlyleaf);

                } else {
                    getChildNodeList(siblingNode, nodeName, nodeList, onlyleaf, (level[0] - 1));
                }
            }
        }
        boolean isLeaf = true;
        while (siblingNode != null) {
            try {
                siblingNode = siblingNode.getNextSibling();
            } catch (IndexOutOfBoundsException e) {
                siblingNode = null;
            }
            // see node as a leaf
            // Here if a comment node inserted before a text node, the text node
            // will not be add into list as leaf node!!!
            if (siblingNode == null && childNode.getNodeType() == Node.TEXT_NODE) {
                if (onlyleaf && isLeaf) {
                    if (nodeName == null || node.getNodeName().equalsIgnoreCase(nodeName)) {
                        nodeList.add(node);
                    }
                }
            } else {
                isLeaf = false;
            }

            if (siblingNode != null && siblingNode.getNodeType() == Node.ELEMENT_NODE) {
                if (nodeName == null && level.length == 0) {
                    nodeList.add(siblingNode);
                }

                if (level.length == 0) {
                    getChildNodeList(siblingNode, nodeName, nodeList, onlyleaf);

                } else {
                    getChildNodeList(siblingNode, nodeName, nodeList, onlyleaf, (level[0] - 1));
                }
            }
            // }

        }
    }
}
