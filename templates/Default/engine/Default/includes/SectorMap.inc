<table class="lmt" style="margin: auto;"><?php
	$GalaxyMap = isset($GalaxyMap)&&$GalaxyMap;
	foreach($MapSectors as &$MapSector) { ?>
		<tr><?php
			foreach($MapSector as &$Sector) {
				$isCurrentSector = $ThisSector->equals($Sector);
				$isLinkedSector = $ThisSector->isLinkedSector($Sector); ?>
				<td>
					<?php if($isCurrentSector){ ?><div class="currentSeclm"><?php } ?>
					<div id="sector<?php echo $Sector->getSectorID(); ?>" class="ajax <?php if($isCurrentSector){ ?>currentSeclm2<?php }else if($isLinkedSector){ ?>connectSeclm<?php }else if($Sector->isVisited($ThisPlayer)){ ?>normalSeclm<?php }else{ ?>normalSeclmu<?php } ?> lm_sector">
						<div class="lmup"><?php
							if($Sector->isVisited($ThisPlayer)) { ?>
								<div class="center lmup"><?php
									if($Sector->getLinkUp()) { ?>
										<img src="images/link_hor.gif" width="12" height="5" alt="Up" title="Up" /><?php
									} ?>
								</div><?php
							}
							if(($Sector->isVisited($ThisPlayer) && ($Sector->hasWarp() || $Sector->hasMine())) || $ThisPlayer->isPartOfCourse($Sector) || ($ThisPlayer->hasPlottedCourse()&&$isCurrentSector)) { ?>
								<div class="lmp"><?php
									if($ThisPlayer->isPartOfCourse($Sector) || ($ThisPlayer->hasPlottedCourse()&&$isCurrentSector)) {
										?><img title="Course" alt="Course" src="images/plot_icon.gif" width="16" height="16"/><?php
									}
									if($Sector->isVisited($ThisPlayer) && $Sector->hasWarp()) {
										if($GalaxyMap){ ?><a href="<?php echo $Sector->getWarpSector()->getGalaxyMapHREF(); ?>"><?php }
										else if($isCurrentSector){ ?><a href="<?php echo $Sector->getWarpSector()->getLocalMapHREF(); ?>"><?php } ?>
											<img title="Warp to #<?php echo $Sector->getWarp(); ?> (<?php echo $Sector->getWarpSector()->getGalaxyName(); ?>)" alt="Warp to #<?php echo $Sector->getWarp(); ?>" src="images/warp.png" width="16" height="16" /><?php
										if($isCurrentSector || $GalaxyMap){ ?></a><?php }
									}
									if($Sector->isVisited($ThisPlayer) && $Sector->hasMine()) {
										?><img src="images/asteroid.gif" width="16" height="16" alt="Mining Available Here" title="Mining Available Here" /><?php
									} ?>
								</div><?php
							}
							$CanScanSector = ($ThisShip->hasScanner() && $isLinkedSector) || $isCurrentSector;
							if( ($CanScanSector && ($Sector->hasForces() || $Sector->hasPlayers()) ) || $Sector->hasFriendlyForces($ThisPlayer) || $Sector->hasFriendlyTraders($ThisPlayer)) { ?>
								<div class="lmtf"><?php
									if(($isCurrentSector && $Sector->hasOtherTraders($ThisPlayer)) || ($ThisShip->hasScanner() && $isLinkedSector && $Sector->hasPlayers()) || $Sector->hasFriendlyTraders($ThisPlayer)) {
										if($CanScanSector && $Sector->hasEnemyTraders($ThisPlayer)) {
											?><img title="Enemy Trader" alt="Enemy Trader" src="images/enemyTrader.png" width="13" height="16"/><?php
										}
										if($Sector->hasFriendlyTraders($ThisPlayer)) {
											?><img title="Friendly Trader" alt="Friendly Trader" src="images/friendlyTrader.png" width="13" height="16"/><?php
										}
									}
									if($Sector->hasForces()) {
										if($CanScanSector && $Sector->hasEnemyForces($ThisPlayer)) {
											?><img title="Enemy Forces" alt="Enemy Forces" src="images/enemyForces.png" width="13" height="16"/><?php
										}
										if($Sector->hasFriendlyForces($ThisPlayer)) {
											?><img title="Friendly Forces" alt="Friendly Forces" src="images/friendlyForces.png" width="13" height="16"/><?php
										}
										/*?><img title="Forces" alt="Forces" src="images/forces.jpg"/><?php*/
									} ?>
								</div><?php
							} ?>
						</div><?php
						if($Sector->isVisited($ThisPlayer)) {
							if($Sector->hasLocation() || $Sector->hasPlanet()) { ?>
								<div class="center lmloc fullwidth"><?php
								if($Sector->hasLocation()) {
									foreach($Sector->getLocations() as $Location) {
										if($isCurrentSector && $Location->hasAction() && !$GalaxyMap) {
											?><a href="<?php echo $Location->getExamineHREF() ?>"><?php
										} ?>
											<img src="<?php echo $Location->getImage() ?>" width="16" height="16" alt="<?php echo $Location->getName() ?>" title="<?php echo $Location->getName() ?>" /><?php
										if($isCurrentSector && $Location->hasAction() && !$GalaxyMap){ ?></a><?php }
									}
								}
								if($Sector->hasPlanet()) {
									if($isCurrentSector && !$GalaxyMap) {
										?><a href="<?php echo $Sector->getPlanet()->getExamineHREF(); ?>"><?php
									} ?>
									<img title="Planet" alt="Planet" src="images/planet.gif" width="16" height="16"/><?php
									if($isCurrentSector && !$GalaxyMap){ ?></a><?php }
								} ?>
								</div><?php
							} ?>
							<div class="left lmleft"><?php
								if($Sector->getLinkLeft()) {
									?><img src="images/link_ver.gif" width="5" height="12" alt="Left" title="Left" /><?php
								} ?>
							</div><?php
						} ?>
						<div class="lmsector center <?php if($Sector->isVisited($ThisPlayer)){ ?>visited<?php }else{ ?>unvisited<?php } ?> fullwidth"><?php echo $Sector->getSectorID(); ?></div><?php
						if($Sector->isVisited($ThisPlayer)) {
							?><div class="right lmright"><?php
								if($Sector->getLinkRight()) {
									?><img src="images/link_ver.gif" width="5" height="12" alt="Right" title="Right" /><?php
								} ?>
							</div><?php
							if(($isCurrentSector && $Sector->hasPort()) || $Sector->hasCachedPort($ThisPlayer)) {
								if($isCurrentSector && $Sector->hasPort()) {
									$Port =& $Sector->getPort();
								}
								else if($Sector->hasCachedPort($ThisPlayer)) {
									$Port =& $Sector->getCachedPort($ThisPlayer);
								} ?>
								<div class="left lmport"><?php
									if($isCurrentSector && !$GalaxyMap) {
										?><a href="<?php echo Globals::getTradeHREF(); ?>"><?php
									} ?>
										<img src="images/port/buy.gif" width="5" height="16" alt="Buy (<?php echo $Port->getRaceName(); ?>)" title="Buy (<?php echo $Port->getRaceName(); ?>)" /><?php
											foreach($Port->getVisibleGoodsSold($ThisPlayer) as $Good) { ?>
												<img src="<?php echo $Good['ImageLink']; ?>" width="13" height="16" title="<?php echo $Good['Name'] ?>" alt="<?php echo $Good['Name']; ?>" /><?php
											} ?><br />
										<img src="images/port/sell.gif" width="5" height="16" alt="Sell (<?php echo $Port->getRaceName(); ?>)" title="Sell (<?php echo $Port->getRaceName(); ?>)" /><?php
											foreach($Port->getVisibleGoodsBought($ThisPlayer) as $Good) { ?>
												<img src="<?php echo $Good['ImageLink']; ?>" width="13" height="16" title="<?php echo $Good['Name'] ?>" alt="<?php echo $Good['Name']; ?>" /><?php
											}
									if($isCurrentSector && !$GalaxyMap){ ?></a><?php } ?>
								</div><?php
								} ?>
							<div class="center lmdown fullwidth"><?php
								if($Sector->getLinkDown()){ ?>
									<img src="images/link_hor.gif" width="12" height="5" alt="Down" title="Down" /><?php
								} ?>
							</div>
						<?php
						}
						if($GalaxyMap) { ?>
							<a href="<?php echo $Sector->getGalaxyMapHREF(); ?>"><img title="" alt="" class="move_hack" src="images/blank.gif" width="120" height="120"/></a><?php
						}
						else if($isLinkedSector) { ?>
							<a href="<?php echo $Sector->getLocalMapHREF(); ?>"><img title="" alt="" class="move_hack" src="images/blank.gif" width="120" height="120"/></a><?php
						}
						else if($isCurrentSector) { ?>
							<a href="<?php echo Globals::getCurrentSectorHREF(); ?>"><img title="" alt="" class="move_hack" src="images/blank.gif" width="120" height="120"/></a><?php
						} ?>
					</div>
					<?php if($isCurrentSector){ ?></div><?php } ?>
				</td><?php
			} unset($Sector); ?>
		</tr><?php
		//A little hacky but clearing these caches for each row of gal map drastically reduces total memory usage, and these caches should never be needed after this point either.
		SmrPort::clearCache();
		SmrForce::clearCache();
		SmrPlanet::clearCache();
		} unset($MapSector); ?>
</table>