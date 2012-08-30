cost = 4
texture = "Temple"
ethnicity = ""

function getName()
	if language == "french" then
		return "Temple"
	else
		return "Temple"
	end
end

function getDescription()
	if language == "french" then
		return "+1 heureux, +1 d'influence et permet de former des prêtres."
	else
		return "+1 happy, +1 influence and allows to train priests."
	end
end

function getMod_happy(val)
	return val + 1
end

function getMod_radius(val)
	return val + 1
end

function getProducedUnits()
	return ethnicity.."Priest", 6
end

function isAllowed(town)
	return townHasBuilding(town, "Shrine")
end
