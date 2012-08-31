icon = "berserker"

function getName()
	if language == "french" then
		return "Berserker"
	else
		return "Berserker"
	end
end

function getDescription()
	if language == "french" then
		return "Pour chaque point de vie perdu, l'unité gagne 1 point de mêlée."
	else
		return "For eacg hit point lost, the unit gain 1 melee."
	end
end

function getMod_Melee(value, params)
	player, unit = splitint(params, " ")
	life, endurance = getUnitBaseData(player, unit, "life", "endurance")
	return value + endurance - life
end
