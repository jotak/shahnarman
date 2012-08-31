icon = "piety"

function getName()
	if language == "french" then
		return "Piété"
	else
		return "Piety"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité inflige 2 dégâts supplémentaires en combat lorsqu'elle affronte une unité de type CHAOS."
	else
		return "The unit deals 2 extra damages in battle when fighting against a CHAOS unit."
	end
end

function onMeleeAttack(unitids)
	player2, unit2 = getDefender()
	life, law, death, chaos = getUnitAlignment(player2, unit2)
	if chaos == 1 then
		damages = getAttackerDamages()
		setAttackerDamages(damages + 2)
	end
end

function onMeleeDefend(unitids)
	player2, unit2 = getAttacker()
	life, law, death, chaos = getUnitAlignment(player2, unit2)
	if chaos == 1 then
		damages = getDefenderDamages()
		setDefenderDamages(damages + 2)
	end
end

function onRangeAttack(unitids)
	player2, unit2 = getDefender()
	life, law, death, chaos = getUnitAlignment(player2, unit2)
	if chaos == 1 then
		damages = getAttackerDamages()
		setAttackerDamages(damages + 2)
	end
end

function onRangeDefend(unitids)
	player2, unit2 = getAttacker()
	life, law, death, chaos = getUnitAlignment(player2, unit2)
	if chaos == 1 then
		damages = getDefenderDamages()
		setDefenderDamages(damages + 2)
	end
end
