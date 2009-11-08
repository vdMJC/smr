<?php
require_once('SmrMySqlDatabase.class.inc');

require_once(get_file_loc('SmrPlayer.class.inc'));
require_once(get_file_loc('SmrLocation.class.inc'));
require_once(get_file_loc('SmrPlanet.class.inc'));
require_once(get_file_loc('SmrPort.class.inc'));
require_once(get_file_loc('SmrGalaxy.class.inc'));
		
class SmrSector
{
	protected static $CACHE_SECTORS = array();
	protected static $CACHE_ALL_SECTORS = array();
	protected static $CACHE_GALAXY_SECTORS = array();
	
	protected $db;
	

	protected $gameID;
	protected $sectorID;
	protected $battles;
	protected $galaxyID;
	protected $visited = array();
	protected $links;
	protected $warp;
	
	protected $hasChanged = false;
	protected $isNew = false;

	public static function &getGalaxySectors($gameID,$galaxyID,$forceUpdate = false)
	{
		if($forceUpdate || !isset(self::$CACHE_GALAXY_SECTORS[$gameID][$galaxyID]))
		{
			$db = new SmrMySqlDatabase();
			$db->query('SELECT sector_id FROM sector WHERE game_id = ' . $gameID .' AND galaxy_id='.$galaxyID.' ORDER BY sector_id ASC');
			$sectors = array();
			while($db->nextRecord())
			{
				$sectorID = $db->getField('sector_id');
				$sectors[$sectorID] =& self::getSector($gameID,$sectorID,$forceUpdate);
			}
			return self::$CACHE_GALAXY_SECTORS[$gameID][$galaxyID] =& $sectors;
		}
		return self::$CACHE_GALAXY_SECTORS[$gameID][$galaxyID];
	}

	public static function &getSector($gameID,$sectorID,$forceUpdate = false)
	{
		if(!isset(self::$CACHE_SECTORS[$gameID][$sectorID]) || $forceUpdate)
		{
			$s = new SmrSector($gameID,$sectorID);
			return self::$CACHE_SECTORS[$s->getGameID()][$s->getSectorID()] =& $s;
		}
		return self::$CACHE_SECTORS[$gameID][$sectorID];
	}
	
	public static function saveSectors()
	{
		foreach(self::$CACHE_SECTORS as &$gameSectors)
		{
			foreach($gameSectors as &$sector)
			{
				$sector->update();
			}
		}
	}
	
	public static function &createSector($gameID,$sectorID)
	{
		if(!isset(self::$CACHE_SECTORS[$gameID][$sectorID]))
		{
			$s = new SmrSector($gameID,$sectorID,true);
			self::$CACHE_SECTORS[$gameID][$sectorID] =& $s;
		}
		return self::$CACHE_SECTORS[$gameID][$sectorID];
	}
	
	protected function __construct($gameID, $sectorID,$create=false)
	{
		$this->db = new SmrMySqlDatabase();

		$this->db->query('SELECT * FROM sector WHERE game_id = '.$gameID.' AND ' .
														'sector_id = '.$sectorID.' LIMIT 1');
		if($this->db->nextRecord())
		{
			$this->sectorID		= (int)$this->db->getField('sector_id');
			$this->gameID		= (int)$this->db->getField('game_id');
			$this->galaxyID		= (int)$this->db->getField('galaxy_id');
			$this->battles		= (int)$this->db->getField('battles');
			if (empty($this->battles)) $this->battles = 0;
	
			$this->links = array();
			if($this->db->getField('link_up'))
				$this->links['Up']=(int)$this->db->getField('link_up');
				
			if($this->db->getField('link_down'))
				$this->links['Down']=(int)$this->db->getField('link_down');
				
			if($this->db->getField('link_left'))
				$this->links['Left']=(int)$this->db->getField('link_left');
				
			if($this->db->getField('link_right'))
				$this->links['Right']=(int)$this->db->getField('link_right');
				
	
			$this->db->query('SELECT * FROM warp WHERE game_id = '.$this->gameID.' AND ' .
												  '(sector_id_1 = '.$this->sectorID.' OR ' .
												   'sector_id_2 = '.$this->sectorID.')');
			if ($this->db->nextRecord())
				$this->warp=($this->db->getField('sector_id_1') == $this->sectorID) ? (int)$this->db->getField('sector_id_2') : (int)$this->db->getField('sector_id_1');
			else
				$this->warp=0;
		}
		else if($create)
		{
			$this->gameID		= (int)$gameID;
			$this->sectorID		= (int)$sectorID;
			$this->battles		= 0;
			$this->links = array();
			$this->isNew		= true;
			return;
		}
		else
		{
			throw new Exception('No such sector: '.$gameID.'-'.$sectorID);
		}
	}

	function update()
	{
		if($this->hasChanged)
		{
			if($this->isNew===false)
			{
				$this->db->query('UPDATE sector SET battles = '.$this->getBattles() .
										', galaxy_id=' . $this->getGalaxyID().
										', link_up=' . $this->getLinkUp().
										', link_right=' . $this->getLinkRight().
										', link_down=' . $this->getLinkDown().
										', link_left=' . $this->getLinkLeft().
								   ' WHERE game_id = '.$this->getGameID().' AND ' .
										 'sector_id = '.$this->getSectorID() . ' LIMIT 1');
			}
			else
			{
				$this->db->query('INSERT INTO sector(sector_id,game_id,galaxy_id,link_up,link_down,link_left,link_right)' .
						'values' .
						'(' . $this->getSectorID() .
						',' . $this->getGameID() .
						',' . $this->getGalaxyID() .
						',' . $this->getLinkUp() .
						',' . $this->getLinkDown() .
						',' . $this->getLinkLeft() .
						',' . $this->getLinkRight() .
						')');
				$this->isNew=false;
			}
		}
	}

	function markVisited(AbstractSmrPlayer &$player)
	{
		if($this->hasPort())
			$this->getPort()->addCachePort($player->getAccountID());

		//now delete the entry from visited
		if(!$this->isVisited($player))
			$this->db->query('DELETE FROM player_visited_sector ' .
					 'WHERE game_id = '.$this->getGameID().' AND sector_id = '.$this->getSectorID().' AND ' .
					 'account_id = '.$player->getAccountID().' LIMIT 1');
		$this->visited[$player->getAccountID()]=true;
	}

	function has_good($good_id, $transaction=false)
	{
		if(!$this->hasPort())
			return false;
		return $this->getPort()->hasGood($good_id,$transaction);
	}

	function has_weapon_shop()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isWeaponSold())
				return true;
		}
		return false;
	}

	function has_hq()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isHQ())
				return true;
		}
		return false;
	}

	function has_ug()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isUG())
				return true;
		}
		return false;
	}

	function has_ship_shop()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isShipSold())
				return true;
		}
		return false;
	}

	function has_fed_beacon()
	{
		return $this->offersFederalProtection();
	}

	function offersFederalProtection()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isFed())
				return true;
		}
		return false;
	}

	function has_bar()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isBar())
				return true;
		}
		return false;
	}

	function has_hardware_shop()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isHardwareSold())
				return true;
		}
		return false;
	}

	function has_bank()
	{
		$locations =& $this->getLocations();
		foreach($locations as &$location)
		{
			if($location->isBank())
				return true;
		}
		return false;
	}

	function enteringSector(AbstractSmrPlayer &$player, $movementType)
	{
		// send scout messages to user
		$message = 'Your forces have spotted ' . $player->getDisplayName() . ' ';
		switch($movementType)
		{
			case MOVEMENT_JUMP:
				$message .= 'jumping into';
			break;
			case MOVEMENT_WARP:
				$message .= 'warping into';
			break;
			case MOVEMENT_WALK:
			default:
				$message .= 'entering';
		}
		$message .= ' sector #'.$this->getSectorID();
		
		$forces =& $this->getForces();
		foreach($forces as &$force)
		{
			$force->ping($message,$player);
		} unset($force);
	}

	function leavingSector(AbstractSmrPlayer &$player, $movementType)
	{
		// send scout messages to user
		$message = 'Your forces have spotted ' . $player->getDisplayName() . ' ';
		switch($movementType)
		{
			case MOVEMENT_JUMP:
				$message .= 'jumping from';
			break;
			case MOVEMENT_WARP:
				$message .= 'warping from';
			break;
			case MOVEMENT_WALK:
			default:
				$message .= 'leaving';
		}
		$message .= ' sector #'.$this->getSectorID();
			
		// iterate over all scout drones in sector
		$forces =& $this->getForces();
		foreach($forces as &$force)
		{
			$force->ping($message,$player);
		} unset($force);
		$this->db->query('UPDATE sector_has_forces SET refresher = 0 WHERE game_id = '.$this->getGameID().' AND sector_id = '.$this->getSectorID().' AND ' . 
					'refresher = '.$player->getAccountID());
	}

	function died_here(AbstractSmrPlayer &$player)
	{
		// iterate over all scout drones in sector
		$forces =& $this->getForces();
		foreach($forces as &$force)
		{
			// send scout messages to user
			$message = 'Your forces have spotted that ' . $player->getDisplayName() . ' has been <font color=red>DESTROYED</font> in sector #'.$this->sectorID;
			$force->ping($message,$player);
		} unset($force);
	}
	
	public function getGameID()
	{
		return $this->gameID;
	}
	
	public function getSectorID()
	{
		return $this->sectorID;
	}
	
	public function getGalaxyID()
	{
		return $this->galaxyID;
	}
	
	public function setGalaxyID($galaxyID)
	{
		if($this->galaxyID==$galaxyID)
			return;
		$this->galaxyID=$galaxyID;
		$this->hasChanged=true;
	}
	
	public function getGalaxyName()
	{
		return $this->getGalaxy()->getName();
	}
	
	public function getNumberOfLinks()
	{
		$num=0;
		if(!is_array($this->getLinks()))
			return $num;
		foreach($this->getLinks() as $link)
			if($link!==0)
				$num++;
		return $num;
	}
	
	public function getNumberOfConnections()
	{
		$links = $this->getNumberOfLinks();
		if($this->hasWarp())
			$links++;
		return $links;
	}
	
	public function &getGalaxy()
	{
		return SmrGalaxy::getGalaxy($this->getGameID(),$this->getGalaxyID());
	}
	
	public function getNeighbourID($dir)
	{
		if($this->hasLink($dir))
			return $this->getLink($dir);
		$galaxy =& $this->getGalaxy();
		$neighbour = $this->getSectorID();
		switch($dir)
		{
			case 'Up':
				$neighbour -= $galaxy->getWidth();
				if($neighbour<$galaxy->getStartSector())
					$neighbour+=$galaxy->getSize();
			break;
			case 'Down':
				$neighbour += $galaxy->getWidth();
				if($neighbour>$galaxy->getEndSector())
					$neighbour-=$galaxy->getSize();
			break;
			case 'Left':
				$neighbour -= 1;
				if((1+$neighbour-$galaxy->getStartSector())%$galaxy->getWidth()==0)
					$neighbour+=$galaxy->getWidth();
			break;
			case 'Right':
				$neighbour += 1;
				if(($neighbour-$galaxy->getStartSector())%$galaxy->getWidth()==0)
					$neighbour-=$galaxy->getWidth();
			break;
			default:
				throw new Exception($dir.': is not a valid direction');
		}
		return $neighbour;
	}
	
	public function &getNeighbourSector($dir)
	{
		return SmrSector::getSector($this->getGameID(),$this->getNeighbourID($dir));
	}
	
	public function getLinks()
	{
		return $this->links;
	}
	
	public function getLink($name)
	{
		return isset($this->links[$name]) ? $this->links[$name] : 0;
	}
	
	public function hasLink($name)
	{
		return $this->getLink($name)!=0;
	}
	
	public function &getLinkSector($name)
	{
		if($this->hasLink($name))
			return SmrSector::getSector($this->getGameID(),$this->getLink($name));
		return false;
	}
	
	/**
	 * Cannot be used for Warps
	 */
	public function setLink($name,$linkID)
	{
		if($this->getLink($name)==$linkID)
			return;
		if($linkID==0)
			unset($this->links[$name]);
		else
			$this->links[$name] = $linkID;
		$this->hasChanged=true;
	}
	
	/**
	 * Cannot be used for Warps
	 */
	public function setLinkSector($dir,SmrSector &$linkSector)
	{
		if($this->getLink($dir)==$linkSector->getSectorID()||$linkSector->equals($this))
			return;
		$this->setLink($dir,$linkSector->getSectorID());
		$linkSector->setLink(self::oppositeDir($dir),$this->getSectorID());
		$this->hasChanged=true;
	}
	/**
	 * Cannot be used for Warps
	 */
	public function enableLink($dir)
	{
		$this->setLinkSector($dir,$this->getNeighbourSector($dir));
	}
	/**
	 * Cannot be used for Warps
	 */
	public function disableLink($dir)
	{
		$this->setLink($dir,0);
		$this->getNeighbourSector($dir)->setLink(self::oppositeDir($dir),0);
	}
	
	protected static function oppositeDir($dir)
	{
		switch($dir)
		{
			case 'Up': return 'Down';
			case 'Down': return 'Up';
			case 'Left': return 'Right';
			case 'Right': return 'Left';
			case 'Warp': return 'Warp';
		}
	}
	
	public function getLinkUp()
	{
		return $this->getLink('Up');
	}
	
	public function setLinkUp($linkID)
	{
		$this->setLink('Up',$linkID);
	}
	
	public function hasLinkUp()
	{
		return $this->hasLink('Up');
	}
	
	public function getLinkDown()
	{
		return $this->getLink('Down');
	}
	
	public function setLinkDown($linkID)
	{
		$this->setLink('Down',$linkID);
	}
	
	public function hasLinkDown()
	{
		return $this->hasLink('Down');
	}
	
	public function getLinkLeft()
	{
		return $this->getLink('Left');
	}
	
	public function hasLinkLeft()
	{
		return $this->hasLink('Left');
	}
	
	public function setLinkLeft($linkID)
	{
		$this->setLink('Left',$linkID);
	}
	
	public function getLinkRight()
	{
		return $this->getLink('Right');
	}
	
	public function hasLinkRight()
	{
		return $this->hasLink('Right');
	}
	
	public function setLinkRight($linkID)
	{
		$this->setLink('Right',$linkID);
	}
	
	public function getWarp()
	{
		return $this->warp;
	}
	
	public function &getWarpSector()
	{
		return SmrSector::getSector($this->getGameID(),$this->getWarp());
	}
	
	public function hasWarp()
	{
		return $this->getWarp()!=0;
	}
	
	public function setWarp(SmrSector &$warp,$justThisObjectData=false)
	{
		if($justThisObjectData===false)
		{
			if($this->hasWarp())
			{
				//Update both possible combinations for the warp
				$this->db->query('UPDATE warp SET sector_id_1 = '.$warp->getSectorID().' WHERE ' .
												'game_id=' . $this->getGameID().' AND '.
												'sector_id_1 = '.$this->getWarp().' AND ' .
												'sector_id_2 = '.$this->getSectorID().' LIMIT 1');
				$this->db->query('UPDATE warp SET sector_id_2 = '.$warp->getSectorID().' WHERE ' .
												'game_id=' . $this->getGameID().' AND '.
												'sector_id_1 = '.$this->getSectorID().' AND ' .
												'sector_id_2 = '.$this->getWarp().' LIMIT 1');
			}
			else
			{
				//Only need to enter once
				$this->db->query('INSERT INTO warp (game_id,sector_id_1,sector_id_2)values('.$this->getGameID().','.$this->getSectorID().','.$warp->getSectorID().')');
			}
			$warp->setWarp($this,true);
		}
		$this->warp = $warp->getSectorID();
	}
	
	public function removeWarp()
	{
		if($this->hasWarp())
		{
			//Update both possible combinations for the warp
			$this->db->query('DELETE FROM warp WHERE ' .
											  '(sector_id_1 = '.$this->getWarp().' AND ' .
											   'sector_id_2 = '.$this->getSectorID().') OR' .
											  '(sector_id_1 = '.$this->getSectorID().' AND ' .
											   'sector_id_2 = '.$this->getWarp().') LIMIT 1');
		}
		unset($this->warp);
	}
	
	public function hasMine()
	{
		//TODO
		return false;
	}

	public function hasPort()
	{
		return $this->getPort()->exists();
	}

	public function &getPort()
	{
		return SmrPort::getPort($this->getGameID(),$this->getSectorID());
	}
	
	public function &createPort()
	{
		return SmrPort::createPort($this->getGameID(),$this->getSectorID());
	}
	
	public function removePort()
	{
		SmrPort::removePort($this->getGameID(),$this->getSectorID());
	}

	public function hasCachedPort(AbstractSmrPlayer &$player)
	{
		return $this->getCachedPort($player)!==false;
	}

	public function &getCachedPort(AbstractSmrPlayer &$player)
	{
		return SmrPort::getCachedPort($this->getGameID(),$this->getSectorID(),$player->getAccountID());
	}
	
	public function hasAnyLocationsWithAction()
	{
		$locations =& SmrLocation::getSectorLocations($this->getGameID(),$this->getSectorID());
		$hasAction = false;
		foreach($locations as $location)
		{
			if($location->hasAction())
				$hasAction = true;
		}
		return $hasAction;
	}
	
	public function hasLocation($locationTypeID=false)
	{
		$locations =& $this->getLocations();
		if(count($locations) == 0)
			return false;
		if($locationTypeID==false)
			return true;
		foreach($locations as &$location)
		{
			if($location->getTypeID()==$locationTypeID)
				return true;
		}
		return false;
	}
	
	public function &getLocations()
	{
		return SmrLocation::getSectorLocations($this->getGameID(),$this->getSectorID());
	}
	
	public function addLocation(SmrLocation &$location)
	{
		$this->db->query('INSERT INTO location (game_id,sector_id,location_type_id)values('.$this->getGameID().','.$this->getSectorID().','.$location->getTypeID().')');
		SmrLocation::getSectorLocations($this->getGameID(),$this->getSectorID(),true);
	}
	
	public function removeAllLocations()
	{
		$this->db->query('DELETE FROM location WHERE game_id='.$this->getGameID().' AND sector_id='.$this->getSectorID());
		SmrLocation::getSectorLocations($this->getGameID(),$this->getSectorID(),true);
	}

	public function hasPlanet()
	{
		return $this->getPlanet()->exists();
	}
	
	public function &getPlanet()
	{
		return SmrPlanet::getPlanet($this->getGameID(),$this->getSectorID());
	}
	
	public function &createPlanet()
	{
		return SmrPlanet::createPlanet($this->getGameID(),$this->getSectorID());
	}
	
	public function removePlanet()
	{
		SmrPlanet::removePlanet($this->getGameID(),$this->getSectorID());
	}

	public function hasForces()
	{
		return count($this->getForces()) > 0;
	}

	public function &getForces()
	{
		require_once(get_file_loc('SmrForce.class.inc'));
		return SmrForce::getSectorForces($this->getGameID(),$this->getSectorID());
	}

	public function hasTraders()
	{
		return count($this->getTraders()) > 0;
	}

	public function hasOtherTraders(AbstractSmrPlayer &$player)
	{
		return count($this->getOtherTraders($player)) > 0;
	}

	public function &getTraders()
	{
		return SmrPlayer::getSectorPlayers($this->getGameID(),$this->getSectorID());
	}

	public function &getFightingTradersAgainstForces(AbstractSmrPlayer &$attackingPlayer, SmrForce &$defendingForces)
	{
		$forceOwner =& $defendingForces->getOwner();
		if($attackingPlayer->forceNAPAlliance($forceOwner))
			throw new Exception('The trader has a force NAP.');
		$fightingPlayers = array();
		$alliancePlayers =& SmrPlayer::getSectorPlayersByAlliances($this->getGameID(),$this->getSectorID(), array($attackingPlayer->getAllianceID()));
		if(count($alliancePlayers) > 0)
			foreach($alliancePlayers as $accountID => $player)
			{
				if($player->canFight())
				{
					if($attackingPlayer->traderAttackForceAlliance($player) && !$player->forceNAPAlliance($forceOwner))
						$fightingPlayers[$accountID] =& $alliancePlayers[$accountID];
				}
			}
		return $fightingPlayers;
	}

	public function &getFightingTradersAgainstPort(AbstractSmrPlayer &$attackingPlayer, SmrPort &$defendingPort)
	{
		$fightingPlayers = array();
		$alliancePlayers =& SmrPlayer::getSectorPlayersByAlliances($this->getGameID(),$this->getSectorID(), array($attackingPlayer->getAllianceID()));
		if(count($alliancePlayers) > 0)
			foreach($alliancePlayers as $accountID => $player)
			{
				if($player->canFight())
				{
					if($attackingPlayer->traderAttackPortAlliance($player))
						$fightingPlayers[$accountID] =& $alliancePlayers[$accountID];
				}
			}
		return $fightingPlayers;
	}

	public function &getFightingTradersAgainstPlanet(AbstractSmrPlayer &$attackingPlayer, SmrPlanet &$defendingPlanet)
	{
		$fightingPlayers = array();
		$alliancePlayers =& SmrPlayer::getSectorPlayersByAlliances($this->getGameID(),$this->getSectorID(), array($attackingPlayer->getAllianceID()));
		if(count($alliancePlayers) > 0)
		{
			$planetOwner =& $defendingPlanet->getOwner();
			foreach($alliancePlayers as $accountID => $player)
			{
				if($player->canFight())
				{
					if($attackingPlayer->traderAttackPlanetAlliance($player) && !$planetOwner->planetNAPAlliance($player))
						$fightingPlayers[$accountID] =& $alliancePlayers[$accountID];
				}
			}
		}
		return $fightingPlayers;
	}

	public function &getFightingTraders(AbstractSmrPlayer &$attackingPlayer, AbstractSmrPlayer &$defendingPlayer, $checkForCloak=false)
	{
		if($attackingPlayer->traderNAPAlliance($defendingPlayer))
			throw new Exception('These traders are NAPed.');
		$fightingPlayers = array('Attackers' => array(), 'Defenders' => array());
		$alliancePlayers =& SmrPlayer::getSectorPlayersByAlliances($this->getGameID(),$this->getSectorID(), array($attackingPlayer->getAllianceID(), $defendingPlayer->getAllianceID()));
		$attackers = array();
		$defenders = array();
		if(count($alliancePlayers) > 0)
			foreach($alliancePlayers as $accountID => $player)
			{
				if($player->canFight())
				{
					if($attackingPlayer->traderAttackTraderAlliance($player) && !$defendingPlayer->traderDefendTraderAlliance($player) && !$defendingPlayer->traderNAPAlliance($player))
						$attackers[] =& $alliancePlayers[$accountID];
					else if($defendingPlayer->traderDefendTraderAlliance($player) && !$attackingPlayer->traderAttackTraderAlliance($player) && !$attackingPlayer->traderNAPAlliance($player) && ($checkForCloak===false || $attackingPlayer->canSee($player)))
						$defenders[] =& $alliancePlayers[$accountID];
				}
			}
		shuffle($attackers);
		foreach($attackers as &$attacker)
		{
			$fightingPlayers['Attackers'][$attacker->getAccountID()] =& $attacker;
		}
		shuffle($defenders);
		foreach($defenders as &$defender)
		{
			$fightingPlayers['Defenders'][$defender->getAccountID()] =& $defender;
		}
		return $fightingPlayers;
	}

	public function &getPotentialFightingTraders(AbstractSmrPlayer &$attackingPlayer)
	{
		$fightingPlayers = array();
		$alliancePlayers =& SmrPlayer::getSectorPlayersByAlliances($this->getGameID(),$this->getSectorID(), array($attackingPlayer->getAllianceID()));
		if(count($alliancePlayers) > 0)
			foreach($alliancePlayers as $accountID => &$player)
			{
				if($player->canFight())
				{
					if($attackingPlayer->traderAttackTraderAlliance($player))
						$fightingPlayers['Attackers'][$accountID] =& $player;
				}
			} unset($player);
		return $fightingPlayers;
	}

	public function &getOtherTraders(AbstractSmrPlayer &$player)
	{
		$players = SmrPlayer::getSectorPlayers($this->getGameID(),$this->getSectorID()); //Do not use & because we unset something and only want that in what we return
		unset($players[$player->getAccountID()]);
		return $players;
	}
	
	public function getBattles()
	{
		return $this->battles;
	}
	
	public function setBattles($amount)
	{
		if($this->battles == $amount)
			return;
		$this->battles = $amount;
		$this->hasChanged = true;
	}
	
	public function decreaseBattles($amount)
	{
		$this->setBattles($this->battles-$amount);
	}
	
	public function increaseBattles($amount)
	{
		$this->setBattles($this->battles+$amount);
	}
	
	function equals(SmrSector $otherSector)
	{
		return $otherSector->getSectorID()==$this->getSectorID()&&$otherSector->getGameID()==$this->getGameID();
	}
	
	function isLinked(SmrSector $otherSector)
	{
		return $otherSector->getGameID()==$this->getGameID()&&($otherSector->getSectorID()==$this->getWarp()||in_array($otherSector->getSectorID(),$this->getLinks()));
	}
	
	public function isVisited(AbstractSmrPlayer &$player=null)
	{
		if($player===null)
			return true;
		if(!isset($this->visited[$player->getAccountID()]))
		{
			$this->db->query('SELECT sector_id FROM player_visited_sector WHERE sector_id = ' . $this->getSectorID() . ' AND account_id=' . $player->getAccountID() . ' AND game_id=' . $this->getGameID() .' LIMIT 1');
			$this->visited[$player->getAccountID()] = !$this->db->nextRecord();
		}
		return $this->visited[$player->getAccountID()];
	}
	
	protected function getHREF($targetPage)
	{
		$container = array();
		$container['url'] = 'sector_move_processing.php';
		$container['target_page'] = $targetPage;
		$container['target_sector'] = $this->getSectorID();
		return SmrSession::get_new_href($container);
	}
	
	public function getLocalMapHREF()
	{
		return $this->getHREF('map_local.php');
	}
	
	public function getCurrentSectorHREF()
	{
		return $this->getHREF('current_sector.php');
	}
	
	public function getGalaxyMapHREF()
	{
		return '?sector_id='.$this->getSectorID();
	}
	
	public function getScanSectorHREF()
	{
		$container = array();
		$container['url'] = 'skeleton.php';
		$container['body'] = 'sector_scan.php';
		$container['target_sector'] = $this->getSectorID();
		return SmrSession::get_new_href($container);
	}
	
	public function hasX(/*Object*/ &$x)
	{
		if($x instanceof SmrSector)
		{
			return $this->equals($x);
		}
		if ($x == 'Port')
		{
			return $this->hasPort();
		}
		if ($x == 'Location')
		{
			return $this->hasLocation();
		}
		if ($x instanceof SmrLocation)
		{
			return $this->hasLocation($x->getTypeID());
		}

		if(is_array($x) && $x['Type']=='Good') //Check if it's possible for port to have X, hacky but nice performance gains
			if ($this->hasPort())
				if ($this->getPort()->hasX($x))
					return true;

		if($x instanceof SmrWeapon || (is_array($x) && ($x['Type']=='Ship'||$x['Type']=='Hardware')) || (is_string($x) && ($x=='Bank'||$x=='Bar'||$x=='Fed'||$x=='HQ'||$x=='UG'||$x=='Hardware'||$x=='Ship'||$x=='Weapon'))) //Check if it's possible for location to have X, hacky but nice performance gains
			if ($this->hasLocation())
			{
				$locations =& $this->getLocations();
				foreach ($locations as &$l)
				{
					if ($l->hasX($x))
						return true;
				}
			}
		return false;
	}
}

?>