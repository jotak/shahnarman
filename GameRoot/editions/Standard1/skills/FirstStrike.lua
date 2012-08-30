icon = "ane"

function getName()
	if language == "french" then
		return "Initiative"
	else
		return "First strike"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité inflige ses dégâts avant ceux de l'ennemi. S'ils sont suffisants pour tuer l'unité adverse, aucun dégât n'est reçu."
	else
		return "The unit deals damages before the enemy. If the damages kill the enemy unit, it doesn't receive any damage."
	end
end

function onBattleResolved(unitids)
	objtype, player, unit = split(unitids, " ")
	attPlayer, attUnit = getAttacker()
	if attPlayer == player and attUnit == unit then
		defLife = getDefenderLife()
		attPow = getAttackerDamages()
		if defLife - attPow <= 0 then
			setDefenderDamages(0)
		end
		return
	end
	defPlayer, defUnit = getDefender()
	if defPlayer == player and defUnit == unit then
		attLife = getAttackerLife()
		defPow = getDefenderDamages()
		if attLife - defPow <= 0 then
			setAttackerDamages(0)
		end
		return
	end
end
