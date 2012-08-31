cost = {0,0,1,0}
icon = "raiseundead"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Ranimer un mort"
	else
		return "Raise undead"
	end
end

function getDescription()
	if language == "french" then
		return "Crée une goule à l'emplacement d'une tombe. La tombe est détruite."
	else
		return "Create an ghoul from a grave. The grave is destroyed."
	end
end

function onCast()
	selectTarget("dead_unit", "")
end

function onResolve(params, caster)
	player, unit = splitint(params, " ")
	state = getUnitStatus(player, unit)
	if state == "dead" then
		x, y = getObjectPosition("unit", player, unit)
		removeUnit(player, unit)
		summon(caster, x, y, "Ghoul")
	end
end
