cost = {0,1,0,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Mains liées"
	else
		return "Linked hands"
	end
end

function getDescription()
	if language == "french" then
		return "Le joueur ciblé ne pioche pas de sort le prochain tour."
	else
		return "Target player doesn't draw any spell next turn."
	end
end

function onCast()
	selectTarget("player", "")
end

function onResolve(player)
	attachToPlayer(player)
end

function getMod_spells_drawn(val)
	return 0
end

function onNewPlayerTurn_step3()	-- need to specify step. Step 3 is the last one: players have drawn their spells
	discardActiveSpell()
end
