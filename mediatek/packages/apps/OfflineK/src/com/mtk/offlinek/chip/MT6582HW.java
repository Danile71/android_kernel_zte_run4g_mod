package com.mtk.offlinek.chip;

import java.util.Locale;

import com.mtk.offlinek.CmdAgent;
import com.mtk.offlinek.FileHandler;
import com.mtk.offlinek.component.DeviceType;

public class MT6582HW extends GenericHW{
	public MT6582HW(){
		super();
		portList.add(2);
		if(mProjectName.toLowerCase(Locale.getDefault()).contains("lte")){
			deviceMap.put(DeviceType.LTE, 2);
		}
		voltageList.add("1125000");
		voltageList.add("1187500");
		voltageList.add("1237500");
		voltageList.add("1281250");
		voltageList.add("1343750");
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
