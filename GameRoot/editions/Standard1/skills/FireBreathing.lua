icon = "ane"

function getName()
	if language == "french" then
		return "Souffle de feu"
	else
		return "Fire breath"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité inflige 2 dégâts supplémentaires lorsqu'elle attaque."
	else
		return "The unit deals 2 extra damages when it attacks."
	end
end

function onMeleeAttack(unitids)
	damages = getAttackerDamages()
	setAttackerDamages(damages + 2)
end
