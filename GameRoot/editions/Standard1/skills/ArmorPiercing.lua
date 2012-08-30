icon = "ane"

function getName()
	if language == "french" then
		return "Armor piercing"
	else
		return "Armor piercing"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité ignore l'armure de l'adversaire lorsqu'elle inflige ses dégâts."
	else
		return "The unit ignores enemy armor when it deals damages."
	end
end

function onRangeAttack(unitids)
	setDefenderArmor(0)
end

function onRangeDefend(unitids)
	setAttackerArmor(0)
end

function onMeleeAttack(unitids)
	setDefenderArmor(0)
end

function onMeleeDefend(unitids)
	setAttackerArmor(0)
end
