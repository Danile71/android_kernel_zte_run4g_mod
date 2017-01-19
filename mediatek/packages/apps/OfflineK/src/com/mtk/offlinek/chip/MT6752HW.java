package com.mtk.offlinek.chip;

import com.mtk.offlinek.FileHandler;
import com.mtk.offlinek.component.DeviceType;

public class MT6752HW extends GenericHW{

	public MT6752HW(){
		super();
		portList.add(3);
		deviceMap.put(DeviceType.WIFI, 3);
		
		voltageList.add("950000");
		voltageList.add("1000000");
		voltageList.add("1050000");
		voltageList.add("1068750");
		voltageList.add("1125000");
		voltageList.add("1181250");
	}

	@Override
	public int setVoltage(String volText) {
		String cmd;
		String devNode;
		long voltage = Long.parseLong(volText);
		int volIndex = 0;
		volIndex = (int) ((voltage-600000)/6250);

		//To do: consult PMIC owner (Anderson Tsai) to confirm the following
		// For VCORE_AO
		devNode = "/sys/devices/platform/mt-pmic/pmic_access";
		cmd = "662 "+ Integer.toHexString(volIndex);
		FileHandler.setDevNode(devNode, cmd);
		cmd = "664 "+ Integer.toHexString(volIndex);
		FileHandler.setDevNode(devNode, cmd);

		// For VLTE
		//cmd = "63C "+ Integer.toHexString(volIndex);
		//FileHandler.setDevNode(devNode, cmd);
		//cmd = "63E "+ Integer.toHexString(volIndex);
		//FileHandler.setDevNode(devNode, cmd);

		return 0;
	}

	
}
