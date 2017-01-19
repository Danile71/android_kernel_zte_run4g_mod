package com.mtk.offlinek.chip;

import java.util.Locale;

import com.mtk.offlinek.CmdAgent;
import com.mtk.offlinek.FileHandler;
import com.mtk.offlinek.component.DeviceType;

public class MT6592HW extends GenericHW{
	public MT6592HW(){
		super();
		if(mProjectName.toLowerCase(Locale.ENGLISH).contains("lte")){
			portList.add(2);
			deviceMap.put(DeviceType.LTE, 2);
		}
		
		portList.add(3);
		deviceMap.put(DeviceType.WIFI, 3);
		
		voltageList.add("950000");
		voltageList.add("1000000");
		voltageList.add("1075000");
		voltageList.add("1125000");
		voltageList.add("1181250");
	}

	@Override
	public int setVoltage(String volText) {
		String devNode;
		String cmd;
		String output;
		long voltage = Long.parseLong(volText);
		int volIndex = 0;
		volIndex = (int) ((voltage-700000)/6250);
		cmd = "ls /sys/devices/platform/mt6333-user/mt6333_access";
		output = CmdAgent.doCommand(cmd);
		if(output.contains("No such")){	//6332 or 6322
			devNode = "/sys/devices/platform/mt-pmic/pmic_access";
			cmd = "21E "+ Integer.toHexString(volIndex);
			FileHandler.setDevNode(devNode, cmd);
			cmd = "220 "+ Integer.toHexString(volIndex);
			FileHandler.setDevNode(devNode, cmd);
		} else {
			devNode = "/sys/devices/platform/mt6333-user/mt6333_access";
			cmd = "6B "+ Integer.toHexString(volIndex);
			FileHandler.setDevNode(devNode, cmd);
			cmd = "6C "+ Integer.toHexString(volIndex);
			FileHandler.setDevNode(devNode, cmd);
		}
		return 0;
	}

	
	

}
