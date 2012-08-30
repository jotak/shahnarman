cost = {0,1,0,0}
icon = "ant"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Peuple économe"
	else
		return "Economic people"
	end
end

function getDescription()
	if language == "french" then
		return "Construit instantanément un grenier dans la ville Keltic ciblée."
	else
		return "Instantly build in granary in target Keltic town."
	end
end

function onCast()
	selectTarget("town", "custom")
end

function onCheckSelect(town)
	name, ethn = getTownData(town)
	if ethn == "Keltics" then
		alreadyHas = townHasBuilding(town, "Keltics_Granary")
		if alreadyHas ~= 1 then
			return 1
		end
	end
	return 0
end

function onResolve(town)
	buildBuilding(town, "Keltics_Granary");
end
