<?php if(isset($Preview)) { ?><table class="standard"><tr><td><?php echo bbifyMessage($Preview); ?></td></tr></table><?php } ?>
<form name="BoxReplyForm" method="POST" action="<?php echo $BoxReplyFormHref; ?>">
	<b>From: </b><span class="admin">Administrator</span><br />
	<b>To: </b><?php echo $Sender->getPlayerName(); ?> a.k.a <?php echo $SenderAccount->getLogin(); ?>
	<br />
	<input type="number" value="<?php if(isset($BanPoints)) { echo htmlspecialchars($BanPoints); } else { ?>0<?php } ?>" name="BanPoints" size="4" /> Points<br />
	<textarea spellcheck="true" name="message" id="InputFields"><?php if(isset($Preview)) { echo $Preview; } ?></textarea><br /><br />
	<input type="submit" name="action" value="Send message" id="InputFields" />&nbsp;<input type="submit" name="action" value="Preview message" id="InputFields" />
</form>