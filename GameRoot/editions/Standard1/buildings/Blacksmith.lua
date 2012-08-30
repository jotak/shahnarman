cost = 4
texture = "Blacksmith"
ethnicity = ""

function getName()
	if language == "french" then
		return "Forgeron"
	else
		return "Blacksmith"
	end
end

function getDescription()
	if language == "french" then
		return "+25% en formation d'unités, +10% de production."
	else
		return "+25% in training units, +10% production."
	end
end

function getMod_prod(val)
	return val * 1.1
end

function getMod_unitprod(val)
	return val * 1.25
end

function isAllowed(town)
	return townHasBuilding(town, "Workshop")
end
