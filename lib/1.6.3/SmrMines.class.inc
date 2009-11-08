<?php
require_once('AbstractSmrCombatWeapon.class.inc');
require_once('SmrMySqlDatabase.class.inc');

class SmrMines extends AbstractSmrCombatWeapon
{
	protected $numberOfMines;
	
	public function __construct($gameTypeID,$numberOfMines)
	{
		$this->gameTypeID = $gameTypeID;
		$this->numberOfMines = $numberOfMines;
		$this->name = 'Mines';
		$this->raceID = 0;
		$this->maxDamage = 20;
		$this->shieldDamage = 20;
		$this->armourDamage = 20;
		$this->accuracy = 100;
		$this->damageRollover = false;
		$this->raidWeapon = false;
	}
	
	public function getNumberOfMines()
	{
		return $this->numberOfMines;
	}
	
	public function getModifiedAccuracy()
	{
		$modifiedAccuracy = $this->getBaseAccuracy();
		return $modifiedAccuracy;
	}
	
	public function getModifiedForceAccuracyAgainstPlayer(SmrForce &$forces,AbstractSmrPlayer &$targetPlayer,$minesAreAttacker=false)
	{
		return $this->getModifiedForceAccuracyAgainstPlayerUsingRandom($forces,$targetPlayer,mt_rand(1,7) * mt_rand(1,7),$minesAreAttacker);
	}
	
	protected function getModifiedForceAccuracyAgainstPlayerUsingRandom(SmrForce &$forces,AbstractSmrPlayer &$targetPlayer, $random,$minesAreAttacker=false)
	{
		$modifiedAccuracy = $this->getModifiedAccuracy();
		$modifiedAccuracy -= $targetPlayer->getLevelID() + $random;
		if($minesAreAttacker)
			$modifiedAccuracy /= SmrSector::getSector($forces->getGameID(),$forces->getSectorID())->getNumberOfConnections();
		
		return max(0,min(100,$modifiedAccuracy));
	}
	
	public function getMaxModifiedForceAccuracyAgainstPlayer(SmrForce &$forces,AbstractSmrPlayer &$targetPlayer,$minesAreAttacker=false)
	{
		return $this->getModifiedForceAccuracyAgainstPlayerUsingRandom($forces,$targetPlayer,49,$minesAreAttacker);
	}
	
	public function &getModifiedDamage()
	{
		$damage = $this->getDamage();
		return $damage;
	}
	
	public function &getModifiedDamageAgainstForces(AbstractSmrPlayer &$weaponPlayer, SmrForce &$forces)
	{
		$return = array('MaxDamage' => 0, 'Shield' => 0, 'Armour' => 0, 'Rollover' => $this->isDamageRollover());
		return $return;
	}
	
	public function &getModifiedDamageAgainstPort(AbstractSmrPlayer &$weaponPlayer, SmrPort &$port)
	{
		$return = array('MaxDamage' => 0, 'Shield' => 0, 'Armour' => 0, 'Rollover' => $this->isDamageRollover());
		return $return;
	}
	
	public function &getModifiedDamageAgainstPlanet(AbstractSmrPlayer &$weaponPlayer, SmrPlanet &$planet)
	{
		$return = array('MaxDamage' => 0, 'Shield' => 0, 'Armour' => 0, 'Rollover' => $this->isDamageRollover());
		return $return;
	}
	
	public function &getModifiedDamageAgainstPlayer(AbstractSmrPlayer &$weaponPlayer, AbstractSmrPlayer &$targetPlayer)
	{
		$return = array('MaxDamage' => 0, 'Shield' => 0, 'Armour' => 0, 'Rollover' => $this->isDamageRollover());
		return $return;
	}
	
	public function &getModifiedPortDamageAgainstPlayer(SmrPort &$port, AbstractSmrPlayer &$targetPlayer)
	{
		$return = array('MaxDamage' => 0, 'Shield' => 0, 'Armour' => 0, 'Rollover' => $this->isDamageRollover());
		return $return;
	}
	
	public function &getModifiedPlanetDamageAgainstPlayer(SmrPlanet &$planet, AbstractSmrPlayer &$targetPlayer)
	{
		$return = array('MaxDamage' => 0, 'Shield' => 0, 'Armour' => 0, 'Rollover' => $this->isDamageRollover());
		return $return;
	}
	
	public function &getModifiedForceDamageAgainstPlayer(SmrForce &$forces, AbstractSmrPlayer &$targetPlayer,$minesAreAttacker=false)
	{
		if(!$this->canShootTraders()) // If we can't shoot traders then just return a damageless array and don't waste resources calculated any damage mods.
		{
			$return = array('MaxDamage' => 0, 'Shield' => 0, 'Armour' => 0, 'Rollover' => $this->isDamageRollover());
			return $return;
		}
		$damage =& $this->getModifiedDamage();
		if($targetPlayer->getShip()->isFederal()) // do 1/2 damage
		{
			$damage['MaxDamage'] /= 2;
			$damage['Shield'] /= 2;
			$damage['Armour'] /= 2;
		}
		$damage['Launched'] = ceil($this->getNumberOfMines() * $this->getModifiedForceAccuracyAgainstPlayer($forces,$targetPlayer,$minesAreAttacker) / 100);
		$damage['MaxDamage'] = ceil($damage['Launched'] * $damage['MaxDamage']);
		$damage['Shield'] = ceil($damage['Launched'] * $damage['Shield']);
		$damage['Armour'] = ceil($damage['Launched'] * $damage['Armour']);
		return $damage;
	}
	
	public function &shootForces(AbstractSmrPlayer &$weaponPlayer, SmrForce &$forces)
	{
		$return = array('Weapon' => &$this, 'TargetForces' => &$forces);
		return $return;
	}
	
	public function &shootPlayer(AbstractSmrPlayer &$weaponPlayer, AbstractSmrPlayer &$targetPlayer)
	{
		$return = array('Weapon' => &$this, 'TargetPlayer' => &$targetPlayer, 'Hit' => false);
		return $return;
	}
	
	public function &shootPlayerAsForce(SmrForce &$forces, AbstractSmrPlayer &$targetPlayer,$minesAreAttacker=false)
	{
		$return = array('Weapon' => &$this, 'TargetPlayer' => &$targetPlayer, 'Hit' => true);
		$this->doForceDamageToPlayer($return, $forces, $targetPlayer);
		$forces->takeMines($return['WeaponDamage']['Launched']);
		return $return;
	}
	
	protected function &doForceDamageToPlayer(array &$return, SmrForce &$forces, AbstractSmrPlayer &$targetPlayer,$minesAreAttacker=false)
	{
		$return['WeaponDamage'] =& $this->getModifiedForceDamageAgainstPlayer($forces,$targetPlayer,$minesAreAttacker);
		$return['ActualDamage'] =& $targetPlayer->getShip()->doMinesDamage($return['WeaponDamage']);

		if($return['ActualDamage']['KillingShot'])
			$return['KillResults'] =& $targetPlayer->killPlayerByForces($forces);
		return $return;
	}
}

?>