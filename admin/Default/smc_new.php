<?
		require_once(get_file_loc('SmrPlanet.class.inc'));
$db2 = new SmrMySqlDatabase();
$db3 = new SmrMySqlDatabase();
$game_id = $_REQUEST['game_id'];
//first get file name
$db->query('SELECT * FROM game WHERE game_id = '.$game_id);
$db->next_record();
$file = $db->f('game_name');
$file .= '.txt';
//we need to make a file for the SMC thing.
header('Content-Type: text/plain; charset=ISO-8859-1'.EOL);
header('Content-Disposition: attachment; filename=$file'.EOL);
header('Content-transfer-encoding: base64'.EOL);

//game heading and info
$PHP_OUTPUT.=('[GAME]'.EOL);
$name = $db->f('game_name');
$id = $db->f('game_id');
$PHP_OUTPUT.=('.$db->escapeString($id=$name'.EOL);

//get races
$PHP_OUTPUT.=('[RACES]'.EOL);
$db->query('SELECT * FROM race ORDER BY race_id');
while ($db->next_record()) {
	$id = $db->f('race_id');
	$name = $db->f('race_name');
	$PHP_OUTPUT.=('R' . $id . '=$name'.EOL);
}

//galaxies
$PHP_OUTPUT.=('[GALAXIES]'.EOL);
$db->query('SELECT galaxy_name, count(sector_id) as num FROM sector NATURAL JOIN galaxy WHERE game_id = '.$game_id.' GROUP BY sector.galaxy_id ORDER BY sector.sector_id');
$i = 1;
while ($db->next_record()) {

	$name = $db->f('galaxy_name');
	$size = sqrt($db->f('num'));
	$PHP_OUTPUT.=('GAL' . $i . '=$name,$size,$size'.EOL);
	$i++;

}

//icons
$PHP_OUTPUT.=('[ICONS]'.EOL);
$PHP_OUTPUT.=('IWood=Wood'.EOL);
$PHP_OUTPUT.=('IFood=Food'.EOL);
$PHP_OUTPUT.=('IOre =Ore'.EOL);
$PHP_OUTPUT.=('IMetl=Metals'.EOL);
$PHP_OUTPUT.=('ISlav=Slaves'.EOL);
$PHP_OUTPUT.=('IText=Textiles'.EOL);
$PHP_OUTPUT.=('IMach=Machinery'.EOL);
$PHP_OUTPUT.=('ICirc=Circuits'.EOL);
$PHP_OUTPUT.=('IWeap=Weapons'.EOL);
$PHP_OUTPUT.=('IComp=Computers'.EOL);
$PHP_OUTPUT.=('ILux =Luxuries'.EOL);
$PHP_OUTPUT.=('INarc=Narcotics'.EOL);
$PHP_OUTPUT.=('IBank=Bank'.EOL);
$PHP_OUTPUT.=('IBar=Bar'.EOL);
$PHP_OUTPUT.=('IWeap=Weapon shop'.EOL);
$PHP_OUTPUT.=('IHard=Hardware shop'.EOL);
$PHP_OUTPUT.=('IShip=Ship shop'.EOL);
$PHP_OUTPUT.=('IRHQ =Race HQ'.EOL);
$PHP_OUTPUT.=('IFHQ =Federal HQ'.EOL);
$PHP_OUTPUT.=('IUHQ =Underground HQ'.EOL);
$PHP_OUTPUT.=('IFed =Federal Beacon'.EOL);
$PHP_OUTPUT.=('ITrad=Trader'.EOL);
$PHP_OUTPUT.=('IWarp=Warp'.EOL);
$PHP_OUTPUT.=('IPlan=Planet'.EOL);

//goods
$PHP_OUTPUT.=('[GOODS]'.EOL);
$db->query('SELECT * FROM good ORDER BY good_id');
while ($db->next_record()) {
	$fmv = $db->f('base_price');
	$name = $db->f('good_name');
	$id = $db->f('good_id');
	//assume it is a nuetral good for now
	$align = '0';
	//get evil goods here
	if ($id == 5 || $id == 9 || $id == 12)
		$align = '-';
	$PHP_OUTPUT.=('G' . $id . '=$name,$fmv,$align'.EOL);
}

//ship properties
$PHP_OUTPUT.=('[SHIP PROPERTIES]'.EOL);
$PHP_OUTPUT.=('SP1=Cost,integer'.EOL);
$PHP_OUTPUT.=('SP2=Holds,potential'.EOL);
$PHP_OUTPUT.=('SP3=Armor,integer'.EOL);
$PHP_OUTPUT.=('SP4=Shields,integer'.EOL);
$PHP_OUTPUT.=('SP5=Combat drones,potential'.EOL);
$PHP_OUTPUT.=('SP6=Scout drones,potential'.EOL);
$PHP_OUTPUT.=('SP7=Mines,potential'.EOL);
$PHP_OUTPUT.=('SP8=Hardpoints,integer'.EOL);
$PHP_OUTPUT.=('SP9=MR,integer'.EOL);
$PHP_OUTPUT.=('SP10=Scanner,bool'.EOL);
$PHP_OUTPUT.=('SP11=Illusion generator,bool'.EOL);
$PHP_OUTPUT.=('SP12=Cloak,bool'.EOL);
$PHP_OUTPUT.=('SP13=Jumpdrive,bool'.EOL);
$PHP_OUTPUT.=('SP14=DCS,bool'.EOL);


//ships
$PHP_OUTPUT.=('[SHIPS]'.EOL);
$db->query('SELECT * FROM ship_type ORDER BY ship_type_id');
while ($db->next_record()) {
	$id = $db->f('ship_type_id');
	$name = $db->f('ship_name');
	$race_id = 'R' . $db->f('race_id');
	$res = $db->f('buyer_restriction');
	if ($res == 1)
		$align = '+';
	elseif ($res == 2)
		$align = '-';
	else
		$align = '0';
	$speed = $db->f('speed');
	$cost = $db->f('cost');
	$hard = $db->f('hardpoint');
	//assuem 10 for now its not implemented
	$mr = 10;
	$db3->query('SELECT * FROM hardware_type ORDER BY hardware_type_id');
	$props = array();
	while ($db3->next_record()) {
		$hard_id = $db3->f('hardware_type_id');
		$db2->query('SELECT * FROM ship_type_support_hardware WHERE ship_type_id = $id ORDER BY hardware_type_id AND hardware_type_id = $hard_id');
		while ($db2->next_record())
			$props[$hard_id] = $db2->f('max_amount');
	}
	$shields = $props[HARDWARE_SHIELDS];
	$armor = $props[HARDWARE_ARMOR];
	$cargo = $props[HARDWARE_CARGO];
	$combat = $props[HARDWARE_COMBAT];
	$scouts = $props[HARDWARE_SCOUT];
	$mines = $props[HARDWARE_MINE];
	$scanner = $props[HARDWARE_SCANNER];
	$cloak = $props[HARDWARE_CLOAK];
	$illus = $props[HARDWARE_ILLUSION];
	$jump = $props[HARDWARE_JUMP];
	$dcs = $props[HARDWARE_DCS];
	$PHP_OUTPUT.=('SHIP' . $id . '=$name,$race_id,$align,$speed,$cost,$cargo,$armor,$shields,$combat,$scouts,$mines,$hard,$mr,$scanner,$illus,$cloak,$jump,$dcs'.EOL);
	
}

//weapons
$PHP_OUTPUT.=('[WEAPONS]'.EOL);
$db->query('SELECT * FROM weapon_type ORDER BY weapon_type_id');
while ($db->next_record()) {
	$id = $db->f('weapon_type_id');
	$name = $db->f('weapon_name');
	$res = $db->f('buyer_restriction');
	if ($res == 1)
		$align = '+';
	elseif ($res == 2)
		$align = '-';
	else
		$align = '0';
	$race_id = 'R' . $db->f('race_id');
	$cost = $db->f('cost');
	$shi_dam = $db->f('shield_damage');
	$arm_dam = $db->f('armor_damage');
	$acc = $db->f('accuracy');
	$power = $db->f('power_level');
	$PHP_OUTPUT.=('WEP' . $id . '=$name,$race_id,$align,$cost,$shi_dam,$arm_dam,$acc,$power'.EOL);
}

//items
$PHP_OUTPUT.=('[ITEMS]'.EOL);
$db->query('SELECT * FROM hardware_type ORDER BY hardware_type_id');
while ($db->next_record()) {
	$name = $db->f('hardware_name');
	$id = $db->f('hardware_type_id');
	$PHP_OUTPUT.=('ITEM' . $id . '=$name'.EOL);
}

//locations & what they sell
$PHP_OUTPUT.=('[LOCATIONS]'.EOL);
$db->query('SELECT * FROM location_type ORDER BY location_type_id');
while ($db->next_record()) {
	//set amount of things it sells to 0 for comma reasons
	$amount = 0;
	$id = $db->f('location_type_id');
	$name = $db->f('location_name');
	$loc_proc = $db->f('location_processor');
	if ($loc_proc == 'shop_weapon.php')
		$icon = 'IWeap';
	elseif ($loc_proc == 'shop_shop.php')
		$icon = 'IShip';
	elseif ($id == 101)
		$icon = 'IFHQ';
	elseif ($id == 102)
		$icon = 'IUHQ';
	elseif ($id == 201)
		$icon = 'IFed';
	elseif ($loc_proc == 'shop_hardware.php')
		$icon = 'IHard';
	elseif ($loc_proc == 'bank_personal.php')
		$icon = 'IBank';
	elseif ($loc_proc == 'bar_opening.php')
		$icon = 'IBar';
	elseif ($loc_proc == 'government.php')
		$icon = 'IRHQ';
	//first part of line
	$PHP_OUTPUT.=('LOC' . $id . '=$name,$icon');
	//now do we have locations
	$db2->query('SELECT * FROM location_sells_hardware WHERE location_type_id = $id');
	while ($db2->next_record()) {
		$hard_id = $db2->f('hardware_type_id');
		$add = 'ITEM' . $hard_id;
		$PHP_OUTPUT.=(',$add');
		$amount += 1;
	}
	$db2->query('SELECT * FROM location_sells_ships WHERE location_type_id = $id');
	while ($db2->next_record()) {
		$hard_id = $db2->f('ship_type_id');
		$add = 'SHIP' . $hard_id;
		$PHP_OUTPUT.=(',$add');
		$amount += 1;
	}
	$db2->query('SELECT * FROM location_sells_weapons WHERE location_type_id = $id');
	while ($db2->next_record()) {
		$hard_id = $db2->f('weapon_type_id');
		$add = 'WEP' . $hard_id;
		$PHP_OUTPUT.=(',$add');
		$amount += 1;
	}
	//do we need a comma?
	if ($amount == 0)
		$PHP_OUTPUT.=(',');
	//next line
	$PHP_OUTPUT.=(EOL);
}

//now sectors
$PHP_OUTPUT.=('[SECTORS]'.EOL);
$db->query('SELECT * FROM sector WHERE game_id = '.$game_id.' ORDER BY sector_id');
while ($db->next_record()) {
	$id = $db->f('sector_id');
	//right now assume they visited now...since we have no ay of telling the last visit
	$now = date('m/d/Y H:i:s', TIME);
	$timestamp = $now;
	$PHP_OUTPUT.=('.$db->escapeString($id=$timestamp,');
	if ($db->f('link_up') > 0)
		$PHP_OUTPUT.=('N');
	if ($db->f('link_right') > 0)
		$PHP_OUTPUT.=('E');
	if ($db->f('link_down') > 0)
		$PHP_OUTPUT.=('S');
	if ($db->f('link_left') > 0)
		$PHP_OUTPUT.=('W');
	$PHP_OUTPUT.=(',');
	$db2->query('SELECT * FROM warp WHERE game_id = '.$game_id.' AND sector_id_1 = $id');
	if ($db2->next_record()) {
		$warp = $db2->f('sector_id_2');
		$PHP_OUTPUT.=('.$db->escapeString($warp');
	}
	$db2->query('SELECT * FROM warp WHERE game_id = '.$game_id.' AND sector_id_2 = $id');
	if ($db2->next_record()) {
		$warp = $db2->f('sector_id_1');
		$PHP_OUTPUT.=('.$db->escapeString($warp');
	}
	$PHP_OUTPUT.=(',');
	$db2->query('SELECT * FROM port WHERE game_id = '.$game_id.' AND sector_id = $id');
	if ($db2->next_record()) {
		$port_race_id = 'R' . $db2->f('race_id');
		$port_lvl = $db2->f('level');
	}
	if (isset($port_race_id)) {
		$PHP_OUTPUT.=('.$db->escapeString($port_race_id:$port_lvl');
		$db3->query('SELECT * FROM port_has_goods WHERE game_id = '.$game_id.' AND sector_id = $id ORDER BY good_id');
		while ($db3->next_record()) {
			$good_id = $db3->f('good_id');
			$trans = $db3->f('transaction');
			if ($trans == 'Buy')
				$PHP_OUTPUT.=('-G' . $good_id);
			else
				$PHP_OUTPUT.=('+G' . $good_id);
		}
	}
	//get rid of the variables so we dont mistake them for next sector
	unset($port_race_id, $port_lvl, $good_id, $trans);
	$PHP_OUTPUT.=(',');
	$db2->query('SELECT * FROM location WHERE game_id = '.$game_id.' AND sector_id = $id');
	$amount = 0;
	while ($db2->next_record()) {
		$loc_id = $db2->f('location_type_id');
		$add = 'LOC' . $loc_id;
		if ($amount > 0)
			$PHP_OUTPUT.=('+');
		$PHP_OUTPUT.=('.$db->escapeString($add');
		$amount += 1;
	}
	$PHP_OUTPUT.=(',');
	$db2->query('SELECT * FROM planet WHERE game_id = '.$game_id.' AND sector_id = $id');
	if ($db2->next_record()) {
		$planet =& SmrPlanet::getPlanet($game_id,$id);
		$level = $planet->level();
		$owner = $planet->owner_id;
		$db2->query('SELECT * FROM player WHERE game_id = '.$game_id.' AND account_id = $owner');
		$db2->next_record();
		$all_id = $db2->f('alliance_id');
		if ($all_id > 0) {
			$db2->query('SELECT * FROM alliance WHERE game_id = '.$game_id.' AND alliance_id = $all_id');
			$db2->next_record();
			$alliance = stripslashes($db2->f('alliance_name'));
		} else
			$alliance = 'None';
		$PHP_OUTPUT.=('.$db->escapeString($level:$alliance');
	}
	$PHP_OUTPUT.=(EOL);
		
}

?>