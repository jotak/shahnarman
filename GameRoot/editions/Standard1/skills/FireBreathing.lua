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
		return "L'unit� inflige 2 d�g�ts suppl�mentaires lorsqu'elle attaque."
	else
		return "The unit deals 2 extra damages when it attacks."
	end
end

function onMeleeAttack(unitids)
	damages = getAttackerDamages()
	setAttackerDamages(damages + 2)
end
