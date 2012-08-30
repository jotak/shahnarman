cost = {1,0,0,0}
icon = "flowering"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Gros piocheur"
	else
		return "Big drawer"
	end
end

function getDescription()
	if language == "french" then
		return "Piochez un sort supplémentaire au prochain tour."
	else
		return "Draw an exrta spell on next turn."
	end
end

function onCast()
end

function onResolve(params)
	attachToPlayer(player)
end

function getMod_spells_drawn(val)
	return val + 1
end

function onNewPlayerTurn()
	discardActiveSpell()
end
